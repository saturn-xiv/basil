# BASIL - A log aggregation and monitoring solution

## Usage

    ```bash
    $ git clone https://github.com/saturn-xiv/basil.git $HOME/workspace/basil
    $ cd $HOME/workspace/basil/
    $ python3 -m venv tmp/python3
    $ source $PWD/tmp/python3/bin/activate

    # Building
    > python -m pip install --upgrade build
    > python -m build

    # Install dependencies
    > python -m pip install -e .
    > PYTHON_GIL=0 python -m basil -h

    # Install for production
    > python -m pip install .
    ```
