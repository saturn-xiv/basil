import queue
import pickle
import logging
import uuid

from sqlalchemy import Engine


from ..models.task import Task as Item

logger = logging.getLogger(__name__)
_queue = queue.Queue(maxsize=0)


class Task:
    def __init__(self, tid: int):
        self.id = str(uuid.uuid4())
        self.tid = id


def put(task: Task):
    logger.info("push shell-run job %d %s", task.id, task.name)
    _queue.put(pickle.dumps(task))
    logger.debug("current queue size %d", _quque.qsize())


def execute(db: Engine):
    while True:
        buf = _queue.get(block=True, timeout=None)
        task = pickle.loads(buf)

        logger.info("receive shell-run job %s %s", task.id,
                    task.name, " ".join(task.args))
        with Session(db) as session:
            now = datetime.now(timezone.utc)
            it = session.scalars(select(Item).where(Item.id == task.tid)).one()
            if it.ran_at is not None:
                raise ValueError(f"task {it.id} is already executed!")
            it.version += 1
            it.ran_at = now
            it.updated_at = now
            session.commit()
        response = subprocess.run(
            [f"jobs/{name}/run.sh"] + task.args, capture_output=True, text=True)
        if response.returncode == 0:
            logger.info("%s\n%s", task.id, response.stdout)
        else:
            logger.error("%s\n%s", task.id, response.stderr)
        logger.info("done(%d), current queue size %d",
                    response.returncode, _quque.qsize())

        with Session(db) as session:
            now = datetime.now(timezone.utc)
            it = session.scalars(select(Item).where(Item.id == task.tid)).one()
            it.version += 1
            it.exit_code = response.returncode
            it.std_out = response.stdout
            it.std_err = response.stderr
            it.finished_at = now
            it.updated_at = now

            session.commit()
