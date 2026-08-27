#!/bin/bash

set -e

export WORK_DIR=$PWD
cd $WORK_DIR/src/basil/
autopep8 --in-place --recursive controllers models worker *.py

exit 0
