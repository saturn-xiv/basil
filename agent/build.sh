#!/bin/bash

set -e

apt update
apt -y upgrade

# Etc/UTC
apt install -y build-essential pkg-config git snmp \
    libboost-all-dev libsnmp-dev libcurlpp-dev libnghttp2-dev libssh-dev libkrb5-dev libldap2-dev libidn2-dev librtmp-dev libpsl-dev libzstd-dev libbrotli-dev libunistring-dev

make

echo "done."
exit 0
