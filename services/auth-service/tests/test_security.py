import os

os.environ.setdefault("JWT_SECRET", "test-secret")

import jwt as pyjwt  # noqa: E402

from app.security import (  # noqa: E402
    hash_password,
    verify_password,
    permissions_for,
    issue_token,
    JWT_SECRET,
)


def test_password_roundtrip():
    stored = hash_password("supersecret")
    assert verify_password("supersecret", stored)


def test_wrong_password_rejected():
    stored = hash_password("supersecret")
    assert not verify_password("wrong", stored)


def test_password_hash_is_salted():
    # Two hashes of the same password must differ (random salt).
    assert hash_password("same") != hash_password("same")


def test_permissions_for_admin_wildcard():
    assert permissions_for(["admin"]) == ["*"]


def test_permissions_for_user():
    perms = permissions_for(["user"])
    assert "catalog:read" in perms
    assert "orders:read" in perms
    assert "orders:write" not in perms


def test_permissions_dedupe_across_roles():
    perms = permissions_for(["user", "orders-manager"])
    # orders:read appears in both roles but should be listed once.
    assert perms.count("orders:read") == 1


def test_issue_token_carries_permissions():
    token, ttl = issue_token("u1", "alice", ["orders-manager"])
    assert ttl > 0
    claims = pyjwt.decode(token, JWT_SECRET, algorithms=["HS256"])
    assert claims["sub"] == "u1"
    assert claims["username"] == "alice"
    assert "orders:write" in claims["permissions"]
    assert claims["iss"] == "auth-service"
