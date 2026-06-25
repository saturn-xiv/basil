#!/bin/bash

set -e

export WORK_DIR=$PWD
export PACKAGE=basil-$(git describe --tags --always --dirty --first-parent)
export TARGET=$WORK_DIR/tmp/$PACKAGE

if [ -d $TARGET ]
then
    rm -r $TARGET
fi

cd $WORK_DIR/agent/

cd $WORK_DIR/
platforms=("x86_64" "aarch64")
for p in "${platforms[@]}"; do
    cargo build --release --target $p-unknown-linux-gnu
    mkdir -p $TARGET/$p/bin
    cp -v target/$p-unknown-linux-gnu/release/basil $TARGET/$p/bin/
done


cd $WORK_DIR/
if [ ! -d node_modules ]
then
    npm install
fi
assets=("bulma/css/bulma.min.css")
for n in "${assets[@]}"; do
    t=$TARGET/node_modules/$(dirname $n)
    mkdir -p $t
    cp -rv node_modules/$n $t/
done

cp -rv assets LICENSE README.md $TARGET/

XZ_OPT=-9 tar -cJf $TARGET.tar.xz  -C $TARGET .
echo "done($PACKAGE)."
exit 0
