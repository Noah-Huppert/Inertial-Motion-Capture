import os, sys, subprocess, getpass, paramiko

# Log helpers
TAG_PROMPT = "[ PROMPT ]"
TAG_INFO   = "[ INFO   ]"
TAG_DEBUG  = "[ DEBUG  ]"
TAG_ERROR  = "[ ERROR  ]"

class Log:
    def log(self, tag, *args):
        message = ""
        for message_part in args:
            message += "{0} ".format(message_part)

        print("{0} {1}".format(tag, message))

    def p(self, *args):
        self.log(TAG_PROMPT, *args)

    def i(self, *args):
        self.log(TAG_INFO, *args)

    def d(self, *args):
        self.log(TAG_DEBUG, *args)

    def e(self, *args):
        self.log(TAG_ERROR, *args)

    def std(self, prefix, std):
        for line in iter(lambda: std.readline(2048), ""):
            print("{0}{1}".format(prefix, line), end="")

    def stds(self, prefix, stdout, stderr):
        self.std(prefix, stderr)
        self.std(prefix, stdout)

log = Log()

# Get commands
commands = sys.argv
commands.pop(0)

command_all = "all" in commands
command_push = "push" in commands
command_compile = "compile" in commands
command_run = "run" in commands

command_ssh = "ssh" in commands

# Get Edison password
edison = {}
edison['host'] = os.environ.get('EDISON_HOST') or "192.168.2.15"
edison['password'] = os.environ.get('EDISON_PASSWORD')

skip_cli_prompt = os.environ.get('EDISON_CLI_SKIP_PROMPT') == "1"

if not skip_cli_prompt or os.environ.get('EDISON_HOST') == None:
    log.p("Enter Edison host (Default: {0})".format(edison['host']))
    edison['host'] = input() or edison['host']
else:
    log.i("Edison host is", edison['host'])


if not skip_cli_prompt or os.environ.get('EDISON_PASSWORD') == None:
    log.p("Enter Edison password {0}".format(
        "(**********)" if edison['password'] else ""
    ))
    edison['password'] = getpass.getpass(prompt="") or edison['password']
else:
    log.i("Edison password is set")

if command_all or command_push or command_compile or command_run:
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.load_host_keys(os.path.expanduser(os.path.join("~", ".ssh", "known_hosts")))
    ssh.connect(edison['host'], username="edison", password=edison['password'])

    if command_push or command_all:
        log.i("Pushing \"imc-server\" to Edison")

        sftp = ssh.open_sftp()

        for (dir_path, dir_names, filenames) in os.walk("imc-server"):
            for filename in filenames:
                local_path = os.path.join(dir_path, filename)
                server_path = os.path.join("/home/edison", local_path)

                sftp.put(local_path, server_path)

        sftp.close()

        log.i("Push success")

    if command_compile or command_all:
        stdin, stdout, stderr = ssh.exec_command("mkdir -p ~/imc-server/build && cd ~/imc-server/build && cmake .. && make")
        log.d("Compile output")
        log.stds("    ", stderr, stdout)

    if command_run or command_all:
        stdin, stdout, stderr = ssh.exec_command("cd /home/edison/imc-server/build && ./imc-server")
        log.d("Run output")
        log.stds("    ", stderr, stdout)

    ssh.close()
else:
    if command_ssh:
        # Check for ssh-pass
        install_sshpass = False

        try:
            sshpass_p = subprocess.Popen(["sshpass", "-V"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            sshpass_p.communicate()

            install_sshpass = sshpass_p.returncode != 0
        except:
            install_sshpass = True

        if install_sshpass:
            log.i("Installing \"sshpass\"")
            os.system("sudo apt-get install sshpass")

        os.system("sshpass -p {0} ssh edison@{1}".format(edison['password'], edison['host']))