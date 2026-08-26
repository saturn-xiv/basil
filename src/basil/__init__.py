import logging
import argparse
import tomllib
import signal
import sys


logger = logging.getLogger(__name__)


def main():
    parser = argparse.ArgumentParser(description="A rbac service(gRPC).")
    parser.add_argument('-c', '--config', default='config.toml')
    parser.add_argument('-p', '--port', type=int, default=8080)
    parser.add_argument('-w', '--worker', action='store_true',
                        help='start a job worker')
    parser.add_argument('-d', '--debug',
                        action='store_true', help='run on debug mode')
    parser.add_argument('-v', '--verbose',
                        action='version', version='2026.8.26')
    args = parser.parse_args()
    logging.basicConfig(
        format='%(asctime)s %(levelname).1s %(message)s', level=logging.DEBUG if args.debug else logging.INFO)
    logger.debug("running on debug mode")

    logger.debug("load configuration from %s", args.config)
    with open(args.config, "rb") as file:
        config = tomllib.load(file)
        launch_http_server(config, args.port, args.workers)


def launch_http_server(config, port, workers):
    addr = f"0.0.0.0:{port}"
    logger.info(
        "start HTTP server on tcp://%s with %d workers", addr, workers)

    def handle_shutdown(signum, frame):
        logger.warning("received signal %d, waiting for shutdown...", signum)
        sys.exit(0)

    signal.signal(signal.SIGTERM, handle_shutdown)
    signal.signal(signal.SIGINT, handle_shutdown)

    logger.info('exited')
