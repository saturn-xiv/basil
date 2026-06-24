# AGENT

## Usage

- Building

```bash
$ podman run --rm -it --events-backend=file --network host -v $(dirname $PWD):/mnt:z ubuntu:jammy
> cd /mnt/agent/
> ./build.sh
```

- Deployment

```bash
sudo apt install snmpd libboost-log1.83.0 libboost-program-options1.83.0 libboost-json1.83.0 libsnmp40t64 libcurlpp0t64
sudo timedatectl set-timezone UTC
```
