#!/bin/bash

set -e

export WORK_DIR=$PWD
export PACKAGE=basil-$(git describe --tags --always --dirty --first-parent)-$(uname -m)
export TARGET=$WORK_DIR/tmp/$PACKAGE

if [ -d $TARGET ]
then
    rm -r $TARGET
fi
mkdir -p $TARGET

cd $WORK_DIR/agent/
make
cp basil-agent $TARGET/

cd $WORK_DIR/
cargo build --release
cp target/release/basil $TARGET/

echo "done($PACKAGE)."
exit 0
