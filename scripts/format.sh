#!/bin/bash

set -e

export WORK_DIR=$PWD
cd $WORK_DIR/src/basil/

# pip install --upgrade autopep8
autopep8 --in-place --recursive controllers models workers *.py

exit 0
