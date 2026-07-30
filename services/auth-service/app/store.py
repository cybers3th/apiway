"""In-memory user store. Swap for a real database by keeping this interface."""
from __future__ import annotations

import threading
import uuid
from dataclasses import dataclass, field

from .security import hash_password, verify_password


@dataclass
class User:
    id: str
    username: str
    password_hash: str
    roles: list[str] = field(default_factory=lambda: ["user"])


class UserStore:
    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._by_username: dict[str, User] = {}

    def create(self, username: str, password: str, roles: list[str] | None = None) -> User:
        with self._lock:
            if username in self._by_username:
                raise ValueError("username already exists")
            user = User(
                id=str(uuid.uuid4()),
                username=username,
                password_hash=hash_password(password),
                roles=roles or ["user"],
            )
            self._by_username[username] = user
            return user

    def authenticate(self, username: str, password: str) -> User | None:
        with self._lock:
            user = self._by_username.get(username)
        if user and verify_password(password, user.password_hash):
            return user
        return None

    def seed_admin(self, username: str, password: str) -> None:
        try:
            self.create(username, password, roles=["admin"])
        except ValueError:
            pass
