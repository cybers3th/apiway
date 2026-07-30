"""Password hashing and JWT issuance for the auth service."""
from __future__ import annotations

import base64
import hashlib
import hmac
import os
import time

import jwt

PBKDF2_ROUNDS = 200_000

JWT_SECRET = os.environ.get("JWT_SECRET", "")
JWT_ISSUER = os.environ.get("JWT_ISSUER", "auth-service")
JWT_TTL_SECONDS = int(os.environ.get("JWT_TTL_SECONDS", "3600"))

# Role -> permissions. RBAC is defined here and baked into every token so the
# gateway can enforce it without a round trip.
ROLE_PERMISSIONS: dict[str, list[str]] = {
    "admin": ["*"],
    "user": ["catalog:read", "orders:read"],
    "orders-manager": ["orders:read", "orders:write"],
}


def permissions_for(roles: list[str]) -> list[str]:
    perms: set[str] = set()
    for role in roles:
        perms.update(ROLE_PERMISSIONS.get(role, []))
    return sorted(perms)


def hash_password(password: str) -> str:
    salt = os.urandom(16)
    dk = hashlib.pbkdf2_hmac("sha256", password.encode(), salt, PBKDF2_ROUNDS)
    return f"{base64.b64encode(salt).decode()}${base64.b64encode(dk).decode()}"


def verify_password(password: str, stored: str) -> bool:
    try:
        salt_b64, dk_b64 = stored.split("$", 1)
        salt = base64.b64decode(salt_b64)
        expected = base64.b64decode(dk_b64)
    except (ValueError, base64.binascii.Error):
        return False
    dk = hashlib.pbkdf2_hmac("sha256", password.encode(), salt, PBKDF2_ROUNDS)
    return hmac.compare_digest(dk, expected)


def issue_token(subject: str, username: str, roles: list[str]) -> tuple[str, int]:
    if not JWT_SECRET:
        raise RuntimeError("JWT_SECRET is not configured")
    now = int(time.time())
    exp = now + JWT_TTL_SECONDS
    payload = {
        "sub": subject,
        "username": username,
        "roles": roles,
        "permissions": permissions_for(roles),
        "iss": JWT_ISSUER,
        "iat": now,
        "exp": exp,
    }
    token = jwt.encode(payload, JWT_SECRET, algorithm="HS256")
    return token, JWT_TTL_SECONDS
