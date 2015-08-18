#!/bin/bash
# Check for "screen" command
{
    screen --version
} &> /dev/null

screen_exit_code=$?

if [ $screen_exit_code != 1 ]; then
    echo "[ERROR] \"screen\" must be installed"
    exit 1
fi

# Check for Edison
if ! test -e /dev/ttyUSB0 ; then
    echo "[ERROR] Could not find Edison on \"/dev/ttyUSB0\""
    exit 2
fi

# Screen into Edison
echo "[INFO] Connecting to the Edison"
echo "[INFO] To exit enter Ctr+a & d"
echo "[INFO] Press any key to continue..."
read
sudo screen /dev/ttyUSB0 115200