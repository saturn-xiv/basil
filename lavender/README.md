# LAVENDER - A logging & SNMP crawler

```bash
$ podman run --rm -it --events-backend=file --network host -v $(dirname $PWD):/mnt:z ubuntu:noble
> cd /mnt/agent/
> ./build.sh
```

## Issues

- Deployment

```bash
sudo apt install libsnmp40t64 libcurl4t64
sudo timedatectl set-timezone UTC
```

- Too many open files

  ```bash
  sysctl -w fs.inotify.max_user_watches=512000
  ```
