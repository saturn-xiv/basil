# AGENT

## Usage

- Building

```bash
$ podman run --rm -it --events-backend=file --network host -v $(dirname $PWD):/mnt:z ubuntu:noble
> cd /mnt/agent/
> ./build.sh
```

- Deployment

```bash
sudo apt install snmpd libsnmp40t64 libcurl4t64
sudo timedatectl set-timezone UTC
```
