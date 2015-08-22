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
if [ -z "$EDISON_HOST" ]; then
    export EDISON_HOST=$1
fi

if [ -z "$EDISON_HOST" ]; then
    export EDISON_HOST=192.168.2.15
fi

# Push
echo "[INFO] Pushing to Edison($EDISON_HOST)"

rsync -a ./imc-server edison@$EDISON_HOST:~