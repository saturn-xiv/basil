import logging
from datetime import datetime, timezone
from typing import List, Optional

from sqlalchemy import String, DateTime, Text, ForeignKey, LargeBinary
from sqlalchemy.orm import Mapped, mapped_column, relationship

from . import Base

logger = logging.getLogger(__name__)


class Job:
    def __init__(self, name: str, args: list[str] = []):
        self.name = name
        self.args = args


class Task(Base):
    __tablename__ = "tasks"
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"))
    name: Mapped[str] = mapped_column(String(255), index=True)
    job: Mapped[bytes] = mapped_column(LargeBinary, index=True)
    ran_at: Mapped[Optional[datetime]] = mapped_column(
        DateTime, nullable=True)
    exit_code: Mapped[int] = mapped_column(nullable=True)
    std_out: Mapped[str] = mapped_column(Text, nullable=True)
    std_err: Mapped[str] = mapped_column(Text, nullable=True)
    finished_at: Mapped[Optional[datetime]] = mapped_column(
        DateTime, nullable=True)
    version: Mapped[int] = mapped_column(default=0)
    updated_at: Mapped[datetime] = mapped_column(DateTime)
    created_at: Mapped[datetime] = mapped_column(DateTime,
                                                 default=lambda: datetime.now(timezone.utc))
    user: Mapped["User"] = relationship(back_populates="tasks")

    def __repr__(self) -> str:
        return f"{self.name!r}<{self.args!r}>"
