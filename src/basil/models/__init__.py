import logging

from sqlalchemy import create_engine, Engine
from sqlalchemy.orm import DeclarativeBase

logger = logging.getLogger(__name__)


class Base(DeclarativeBase):
    pass


def open_db(debug: bool) -> Engine:
    file = "tmp/.db"
    logger.debug("open database %s", file)
    db = create_engine(f"sqlite:///{file}", echo=debug)
    Base.metadata.create_all(db)
    return db
