#!/bin/bash

set -e

export WORK_DIR=$PWD
export PACKAGE=basil-$(git describe --tags --always --dirty --first-parent)
export TARGET=$WORK_DIR/tmp/$PACKAGE

if [ -d $TARGET ]
then
    rm -r $TARGET
fi

cd $WORK_DIR/
export LAVENDER_BUILD_DIR=$WORK_DIR/lavender/build
cmake -Wno-dev -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DCPR_USE_SYSTEM_CURL=ON -DBUILD_SHARED_LIBS=OFF -DCPR_BUILD_TESTS=OFF \
    -DINJA_BUILD_TESTS=OFF \
    -B $LAVENDER_BUILD_DIR -S $WORK_DIR/lavender \
    -G Ninja
cmake --build $LAVENDER_BUILD_DIR
mkdir -p $TARGET/$(uname -m)/bin
cp $LAVENDER_BUILD_DIR/lavender $TARGET/$(uname -m)/bin/

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
