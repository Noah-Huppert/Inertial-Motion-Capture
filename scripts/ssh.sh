#!/bin/bash

# Check for host option
if [ -z "$EDISON_HOST" ]; then
    export EDISON_HOST=$1
fi

if [ -z "$EDISON_HOST" ]; then
    export EDISON_HOST=192.168.2.15
fi

# SSH
ssh edison@$EDISON_HOST