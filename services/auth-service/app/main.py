"""Authentication service: registers users and issues signed JWTs.

It sits behind the gateway on the public /auth prefix. The gateway validates the
tokens this service mints, so both share the JWT_SECRET.
"""
from __future__ import annotations

import os

from fastapi import Depends, FastAPI, Header, HTTPException, status
from pydantic import BaseModel, Field

from .security import ROLE_PERMISSIONS, issue_token, permissions_for
from .store import UserStore

app = FastAPI(title="Auth Service", version="1.0.0")
store = UserStore()


@app.on_event("startup")
def _seed() -> None:
    admin_user = os.environ.get("ADMIN_USERNAME", "admin")
    admin_pass = os.environ.get("ADMIN_PASSWORD", "admin-change-me")
    store.seed_admin(admin_user, admin_pass)


class RegisterRequest(BaseModel):
    username: str = Field(min_length=3, max_length=64)
    password: str = Field(min_length=8, max_length=256)
    roles: list[str] | None = None


class LoginRequest(BaseModel):
    username: str
    password: str


class TokenResponse(BaseModel):
    access_token: str
    token_type: str = "Bearer"
    expires_in: int
    roles: list[str]
    permissions: list[str]


@app.get("/healthz")
def healthz() -> dict:
    return {"status": "ok"}


@app.post("/auth/register", response_model=TokenResponse, status_code=status.HTTP_201_CREATED)
def register(req: RegisterRequest) -> TokenResponse:
    requested = req.roles or ["user"]
    unknown = [r for r in requested if r not in ROLE_PERMISSIONS]
    if unknown:
        raise HTTPException(status.HTTP_400_BAD_REQUEST, f"unknown roles: {unknown}")
    try:
        user = store.create(req.username, req.password, roles=requested)
    except ValueError as exc:
        raise HTTPException(status.HTTP_409_CONFLICT, str(exc))
    token, ttl = issue_token(user.id, user.username, user.roles)
    return TokenResponse(
        access_token=token, expires_in=ttl,
        roles=user.roles, permissions=permissions_for(user.roles),
    )


@app.post("/auth/login", response_model=TokenResponse)
def login(req: LoginRequest) -> TokenResponse:
    user = store.authenticate(req.username, req.password)
    if not user:
        raise HTTPException(status.HTTP_401_UNAUTHORIZED, "invalid credentials")
    token, ttl = issue_token(user.id, user.username, user.roles)
    return TokenResponse(
        access_token=token, expires_in=ttl,
        roles=user.roles, permissions=permissions_for(user.roles),
    )


@app.get("/auth/me")
def me(
    x_auth_username: str | None = Header(default=None),
    x_auth_roles: str | None = Header(default=None),
    x_auth_permissions: str | None = Header(default=None),
) -> dict:
    """Echoes the identity the gateway resolved from the caller's token."""
    if not x_auth_username:
        raise HTTPException(status.HTTP_401_UNAUTHORIZED, "not authenticated")
    return {
        "username": x_auth_username,
        "roles": (x_auth_roles or "").split(",") if x_auth_roles else [],
        "permissions": (x_auth_permissions or "").split(",") if x_auth_permissions else [],
    }
