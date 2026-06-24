#!/bin/bash

set -e

apt update
apt upgrade

apt install lsb-release wget software-properties-common gnupg \
    snmp build-essential pkg-config git libboost-all-dev libsnmp-dev libcurlpp-dev

# https://apt.llvm.org/
bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
update-alternatives --install /usr/bin/clang clang /usr/bin/clang-22 100
update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-22 100
apt install libc++-22-dev libc++abi-22-dev

make

echo "done."
exit 0
