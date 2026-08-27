import logging
import argparse
import tomllib
import signal
import sys
from logging.handlers import RotatingFileHandler
from datetime import datetime, timezone

from sqlalchemy import select
from sqlalchemy.orm import Session


from .models import open_db
from .models.user import User
from .models.log import Log
from .models.task import Task

logger = logging.getLogger(__name__)


def main():
    parser = argparse.ArgumentParser(description="A log aggregation and monitoring solution.",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        '-c', '--config', default='config.toml', help='configuration file')
    parser.add_argument('-d', '--debug',
                        action='store_true', help='run on debug mode')
    parser.add_argument('-v', '--verbose',
                        action='version', version='2026.8.26')
    subparsers = parser.add_subparsers(dest="Sub-Commands", required=True)

    server_parser = subparsers.add_parser(
        "server", help="start a HTTP server with email/job worker")
    server_parser.add_argument('-p', '--port', type=int, default=8080)
    server_parser.set_defaults(func=_handle_server)

    list_user_parser = subparsers.add_parser(
        "list-user", help="list all users")
    list_user_parser.set_defaults(func=_handle_list_user)

    create_user_parser = subparsers.add_parser(
        "create-user", help="create a new user")
    create_user_parser.add_argument("-e", "--email", required=True)
    create_user_parser.add_argument("-p", "--password", required=True)
    create_user_parser.add_argument("-n", "--name", required=True)
    create_user_parser.set_defaults(func=_handle_create_user)

    set_user_password_parser = subparsers.add_parser(
        "set-user-password", help="set password for user")
    set_user_password_parser.add_argument("-e", "--email", required=True)
    set_user_password_parser.add_argument("-p", "--password", required=True)
    set_user_password_parser.set_defaults(func=_handle_set_user_password)

    enable_user_parser = subparsers.add_parser(
        "enable-user", help="enable a user")
    enable_user_parser.add_argument("-e", "--email", required=True)
    enable_user_parser.set_defaults(func=_handle_enable_user)

    disable_user_parser = subparsers.add_parser(
        "disable-user", help="disable a user")
    disable_user_parser.add_argument("-e", "--email", required=True)
    disable_user_parser.set_defaults(func=_handle_disable_user)

    args = parser.parse_args()
    logging.basicConfig(
        format='%(asctime)s %(levelname).1s %(message)s', level=logging.DEBUG if args.debug else logging.INFO)

    handler = RotatingFileHandler(
        "tmp/log", maxBytes=100 * 1024 * 1024, backupCount=10)
    formatter = logging.Formatter("%(asctime)s %(levelname).1s %(message)s")
    handler.setFormatter(formatter)
    logger.addHandler(handler)

    logger.debug("running on debug mode")
    logger.debug("load configuration from %s", args.config)
    args.func(args)

    #
    # with open(args.config, "rb") as file:
    #     config = tomllib.load(file)
    #     launch_http_server(config, args.port, args.workers)


def _handle_server(args):
    # logger.info("", extra=args)
    print(vars(args))
    pass

    # addr = f"0.0.0.0:{port}"
    # logger.info(
    #     "start HTTP server on tcp://%s with %d workers", addr, workers)

    # def handle_shutdown(signum, frame):
    #     logger.warning("received signal %d, waiting for shutdown...", signum)
    #     sys.exit(0)

    # signal.signal(signal.SIGTERM, handle_shutdown)
    # signal.signal(signal.SIGINT, handle_shutdown)

    # logger.info('exited')


def _handle_create_user(args):
    email = User.to_email(args.email)
    name = User.to_name(args.name)
    password = User.to_password(args.password)
    logger.info("create user %s<%s>", name, email)
    db = open_db(args.debug)
    with Session(db) as session:
        if session.execute(select(User).where(User.email == email)).scalar_one_or_none() is not None:
            raise ValueError(f"user {email} already exists!")
        session.add(User(name=name, email=email, password=password,
                    updated_at=datetime.now(timezone.utc)))
        user = session.scalars(select(User).where(User.email == email)).one()
        session.add(Log(user_id=user.id, message="Created by administrator."))
        session.commit()
    logger.info('done.')


def _handle_set_user_password(args):
    email = User.to_email(args.email)
    password = User.to_password(args.password)
    logger.info("reset user %s's password",  email)
    db = open_db(args.debug)
    with Session(db) as session:
        user = session.scalars(select(User).where(User.email == email)).one()
        user.password = password
        user.version += 1
        user.updated_at = datetime.now(timezone.utc)
        session.add(
            Log(user_id=user.id, message="Reset password by administrator."))
        session.commit()
    logger.info('done.')


def _handle_disable_user(args):
    email = User.to_email(args.email)
    now = datetime.now(timezone.utc)
    logger.info("disable user",  email)
    db = open_db(args.debug)
    with Session(db) as session:
        user = session.scalars(select(User).where(User.email == email)).one()
        if user.deleted_at is not None:
            raise ValueError(f"user {user} is already disabled!")
        user.deleted_at = now
        user.version += 1
        user.updated_at = now
        session.add(Log(user_id=user.id, message="Disabled by administrator."))
        session.commit()
    logger.info('done.')


def _handle_enable_user(args):
    email = User.to_email(args.email)
    logger.info("enable user %s",  email)
    db = open_db(args.debug)
    with Session(db) as session:
        user = session.scalars(select(User).where(User.email == email)).one()
        if user.deleted_at is None:
            raise ValueError(f"user {email} is already enabled!")
        user.deleted_at = None
        user.version += 1
        user.updated_at = datetime.now(timezone.utc)
        session.add(Log(user_id=user.id, message="Enabled by administrator."))
        session.commit()
    logger.info('done.')


def _handle_list_user(args):
    db = open_db(args.debug)
    with Session(db) as session:
        items = session.execute(
            select(User).order_by(User.name)).scalars().all()
        print(f"{'USER':<63} DELETED AT")
        for it in items:
            print(f"{str(it):<63} {it.deleted_at or 'n/a'}")
