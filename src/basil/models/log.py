import logging
from datetime import datetime, timezone
from typing import Optional

from sqlalchemy import Text, DateTime, ForeignKey
from sqlalchemy.orm import Mapped, mapped_column, relationship


from . import Base

logger = logging.getLogger(__name__)


class Log(Base):
    __tablename__ = "logs"
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"))
    message: Mapped[str] = mapped_column(Text)
    created_at: Mapped[datetime] = mapped_column(
        DateTime, default=lambda: datetime.now(timezone.utc))
    user: Mapped["User"] = relationship(back_populates="logs")

    def __repr__(self) -> str:
        return f"created-at={self.created_at!r}, message=<{self.message!r}>"
