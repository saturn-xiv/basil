import logging
from datetime import datetime, timezone
from typing import List, Optional

from sqlalchemy import String, DateTime, ForeignKey
from sqlalchemy.orm import Mapped, mapped_column, relationship

from . import Base

logger = logging.getLogger(__name__)


class Task(Base):
    __tablename__ = "tasks"
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"))
    name: Mapped[str] = mapped_column(String(31), index=True)
    args: Mapped[str] = mapped_column(String(255), index=True)
    ran_at: Mapped[Optional[datetime]] = mapped_column(
        DateTime, nullable=True)
    finished_at: Mapped[Optional[datetime]] = mapped_column(
        DateTime, nullable=True)
    updated_at: Mapped[datetime] = mapped_column(DateTime)
    created_at: Mapped[datetime] = mapped_column(DateTime,
                                                 default=lambda: datetime.now(timezone.utc))
    user: Mapped["User"] = relationship(back_populates="tasks")

    def __repr__(self) -> str:
        return f"{self.name!r}<{self.args!r}>"
