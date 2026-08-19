#!/bin/bash

set -e

export VCPKG_DISABLE_METRICS=1
export VCPKG_DEFAULT_BINARY_CACHE=$PWD/.cache
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

mkdir -p $VCPKG_DEFAULT_BINARY_CACHE
declare -a targets=("x86_64" "aarch64" "riscv64")
for i in "${targets[@]}"
do
   cmake --preset=$i
   cmake --build build/$i
done

cd $WORK_DIR/dashboard/
if [ ! -d node_modules ]
then
    npm install
fi
npm run build
cp -r dist $TARGET_DIR/$PACKAGE/dashboard

cd $WORK_DIR/
cp -r LICENSE README.md $TARGET_DIR/$PACKAGE/

XZ_OPT=-9 tar -cJf $TARGET_DIR/$PACKAGE.tar.xz --remove-files -C $TARGET_DIR/$PACKAGE .
md5sum $TARGET_DIR/$PACKAGE.tar.xz > $TARGET_DIR/$PACKAGE.md5

echo "done($PACKAGE)."
exit 0
