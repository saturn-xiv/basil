#!/bin/bash

set -e

export WORK_DIR=$PWD
export PROTOBUF_HOME=$HOME/local/protobuf

rm -r $WORK_DIR/gourd/src $WORK_DIR/gourd/include
mkdir $WORK_DIR/gourd/src $WORK_DIR/gourd/include
$WORK_DIR/vcpkg/packages/protobuf_x64-linux-release/tools/protobuf/protoc \
    -I $WORK_DIR/proto -I $WORK_DIR/vcpkg/packages/protobuf_x64-linux-release/include/google/protobuf \
    --cpp_out=$WORK_DIR/gourd/src --grpc_out=$WORK_DIR/gourd/src \
    --plugin=protoc-gen-grpc=$WORK_DIR/vcpkg/packages/grpc_x64-linux/tools/grpc/grpc_cpp_plugin \
    $WORK_DIR/proto/*.proto
mv $WORK_DIR/gourd/src/*.h $WORK_DIR/gourd/include/

echo 'done.'
exit 0
