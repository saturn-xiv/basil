#!/bin/bash

set -e

export WORK_DIR=$PWD
export PACKAGE=basil-$(git describe --tags --always --dirty --first-parent)
export TARGET_DIR=$WORK_DIR/tmp/$PACKAGE

source /etc/os-release

if [ -f $TARGET_DIR/$PACKAGE.md5 ]
then
    echo "building $PACKAGE already exists"
    exit 0
fi

if [ -f $TARGET_DIR/$PACKAGE.tar.xz ]
then
    rm $TARGET_DIR/$PACKAGE.tar.xz
fi

if [ -d $TARGET_DIR/$PACKAGE ]
then
    rm -r $TARGET_DIR/$PACKAGE
fi

if [ "$ID" = "ubuntu" ]
then
    apt install -y libboost-all-dev libsnmp-dev libcurl4-openssl-dev
fi

cd $WORK_DIR/
export LAVENDER_BUILD_DIR=$WORK_DIR/lavender/build
cmake -Wno-dev -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DCPR_USE_SYSTEM_CURL=ON -DBUILD_SHARED_LIBS=OFF -DCPR_BUILD_TESTS=OFF \
    -DINJA_BUILD_TESTS=OFF \
    -B $LAVENDER_BUILD_DIR -S $WORK_DIR/lavender \
    -G Ninja
cmake --build $LAVENDER_BUILD_DIR
mkdir -p $TARGET_DIR/$PACKAGE/$(uname -m)/bin
cp $LAVENDER_BUILD_DIR/lavender $TARGET_DIR/$PACKAGE/$(uname -m)/bin/

cd $WORK_DIR/
declare -a platforms=("x86_64" "aarch64" "riscv64gc")
for p in "${platforms[@]}"; do
    cargo build --release --target $p-unknown-linux-gnu
    mkdir -p $TARGET_DIR/$PACKAGE/$p/bin
    cp -v target/$p-unknown-linux-gnu/release/basil $TARGET_DIR/$PACKAGE/$p/bin/
done


cd $WORK_DIR/dashboard/
if [ ! -d node_modules ]
then
    npm install
fi
npm run build
cp -r dist $TARGET_DIR/$PACKAGE/dashboard

cp -r LICENSE README.md $TARGET_DIR/$PACKAGE/

XZ_OPT=-9 tar -cJf $TARGET_DIR/$PACKAGE.tar.xz --remove-files -C $TARGET_DIR/$PACKAGE .
md5sum $TARGET_DIR/$PACKAGE.tar.xz > $TARGET_DIR/$PACKAGE.md5

echo "done($PACKAGE)."
exit 0
