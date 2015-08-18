#!/bin/bash
# Check for RSync
{
    rsync --version
} &> /dev/null

rsync_exit_code=$?

if [[ $rsync_exit_code != 0 ]]; then
    echo "[ERROR] \"rsync\" must be installed"
    exit 1
fi

# Check for host option
if [[ $# != 1 ]]; then
    echo "[ERROR] Edison host must be the first and only argument provided"
fi

# Push
echo "[INFO] Pushing to Edison"

rsync -a ./imc-server root@$1:~/imc-server