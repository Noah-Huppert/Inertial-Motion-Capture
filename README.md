# Inertial-Motion-Capture
Motion capture using Inertial Motion Units

Using [Ubilinux](https://emutex.com/ubilinux) for OS (Deprecated, basically Ubuntu for Intel Edison)

# Usefull Intel Edison commands
- `client`(Any computer)
    - `python cli.py <commands>`
        - Dependencies
            - `paramiko`
        - Run using Python 3(Ubuntu use `python3` in place of `python`)
        - You can put one or more of the following commands in place of
        `<commands>`
            - `push`
                - Pushes the `imc-server` folder to the Intel Edison
            - `compile`
                - Compiles the `imc-server` code on the Intel Edison
            - `run`
                - Runs the `imc-server` executable on the Intel Edison
        - Environment Variables
            - `EDISON_HOST`
                - The address of the Edison
            - `EDISON_PASSWORD`
                - The password for the Edison
            - `EDISON_CLI_SKIP_PROMPT`
                - If `1` the CLI will skip any prompts that are set by
                environment variables
- `host`(Intel Edison)
    - Configure Wifi on Edison
        - `su`
        - `chmod 0600 /etc/network/interfaces`
        - `wpa_passphrase <SSID> <PASSWORD>`
            - Save the `psk` value in the output. It will later be refereed to
            as `<PSK`>
        - Open up `/etc/network/interfaces` in an editor
            - Comment out the line that says `auto usb0`
            - Uncomment the line that says `auto wlan0`
            - Replace `wpa-ssid` with the networks SSID
            - Replace the `wpa-psk` with `<PSK>`
        - `ifup wlan0`
            - This restarts the network adapter
