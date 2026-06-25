#!/bin/bash

set -e

apt update
apt -y upgrade

apt install -y build-essential pkg-config ninja-build cmake git

curl -fsSL https://xmake.io/shget.text | bash
source $HOME/.xmake/profile
export XMAKE_ROOT=y

xmake f -y -p linux -m release
xmake

echo "done."
exit 0
