#!/bin/bash

set -e

export WORK_DIR=$PWD
export TARGET=$WORK_DIR/tmp/basil-$(git describe --tags --always --dirty --first-parent)-$(uname -m)

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

echo "done."
exit 0
