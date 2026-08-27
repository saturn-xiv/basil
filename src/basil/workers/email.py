import queue
import pickle
import logging
import uuid
import smtplib
from email.message import EmailMessage
from email.utils import formataddr

from ..models.user import User

logger = logging.getLogger(__name__)
_queue = queue.Queue(maxsize=0)


class Task:
    def __init__(self, to: User, subject: str, content: str, html: bool = False, cc: list[User] = [], bcc: list[str] = []):
        self.id = str(uuid.uuid4())
        self.to = formataddr((user.name, user.email))
        self.cc = list(map(lambda x: formataddr((x.name, x.email)), cc))
        self.bcc = list(map(lambda x: formataddr((x.name, x.email)), bcc))
        self.subject = subject
        self.content = content


def put(task: Task):
    logger.info("push email-send-job %s %s '%s' into queue",
                task.id, task.to, task.subject)
    _queue.put(pickle.dumps(task))
    logger.debug("current queue size %d", _quque.qsize())


def execute(name: str, email: str, password: str, host: str = "smtp.gmail.com", port: int = 587):
    while True:
        buf = _queue.get(block=True, timeout=None)
        task = pickle.loads(buf)
        logger.info("receive email-send job %s", task.id)
        msg = EmailMessage()
        msg["Subject"] = task.subject
        msg["From"] = formataddr((name, email))
        msg["To"] = task.to
        if task.html:
            msg.add_alternative(task.content, subtype="html")
        else:
            msg.set_content(task.content)

        recipients = [to] + task.cc + task.bcc
        with smtplib.SMTP(host, port) as server:
            server.starttls()
            server.login(email, password)
            server.send_message(msg, to_addrs=recipients)

        logger.info("done, current queue size %d", _quque.qsize())
