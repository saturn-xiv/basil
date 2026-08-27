import logging
import re
from datetime import datetime, timezone
from typing import List, Optional

from sqlalchemy import String, DateTime
from sqlalchemy.orm import Mapped, mapped_column, relationship
import nacl.pwhash

from . import Base

logger = logging.getLogger(__name__)


class User(Base):
    __tablename__ = "users"
    id: Mapped[int] = mapped_column(primary_key=True)
    name: Mapped[str] = mapped_column(String(31), index=True)
    email: Mapped[str] = mapped_column(String(63), unique=True)
    password: Mapped[str] = mapped_column(String(127))
    version: Mapped[int] = mapped_column(default=0)
    deleted_at: Mapped[Optional[datetime]] = mapped_column(
        DateTime, nullable=True)
    updated_at: Mapped[datetime] = mapped_column(DateTime)
    created_at: Mapped[datetime] = mapped_column(DateTime,
                                                 default=lambda: datetime.now(timezone.utc))
    logs: Mapped[List["Log"]] = relationship(
        back_populates="user", cascade="all, delete-orphan")
    tasks: Mapped[List["Task"]] = relationship(
        back_populates="user", cascade="all, delete-orphan")

    @staticmethod
    def to_email(s: str) -> str:
        s = s.strip().lower()
        l = len(s)
        if l < 5 or l > 63:
            raise ValueError("Invalid email length")
        if re.match(r'^[a-z0-9_.+-]+@[a-z0-9-]+\.[a-z0-9-.]+$', s) is None:
            raise ValueError("Invalid email address")
        return s

    @staticmethod
    def to_name(s: str) -> str:
        s = s.strip()
        l = len(s)
        if l < 2 or l > 31:
            raise ValueError("Invalid username")
        return s

    @staticmethod
    def to_password(s: str) -> str:
        l = len(s)
        if l < 6 or l > 31:
            raise ValueError("Invalid password")
        return nacl.pwhash.str(s.encode('utf-8')).decode("utf-8")

    def verify_password(self, s: str):
        if not nacl.pwhash.verify(self.password.encode('utf-8'), s.encode('utf-8')):
            raise ValueError("Invalid password")

    def __repr__(self) -> str:
        return f"name={self.name!r}, email={self.email!r}"

    def __str__(self) -> str:
        return f"{self.name}<{self.email}>"
