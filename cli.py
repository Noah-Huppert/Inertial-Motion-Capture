import os, sys, getpass, paramiko

# Log helpers
TAG_PROMPT = "[ PROMPT ]"
TAG_INFO   = "[ INFO   ]"
TAG_DEBUG  = "[ DEBUG  ]"

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

    def stdout(self, prefix, stdout):
        for line in iter(lambda: stdout.readline(2048), ""):
            print("{0}{1}".format(prefix, line), end="")

log = Log()

# Get commands
commands = sys.argv
commands.pop(0)

# Get Edison password
edison = {}
edison['host'] = os.environ.get('EDISON_HOST') or "192.168.2.15"
edison['password'] = os.environ.get('EDISON_PASSWORD')

if os.environ.get('EDISON_CLI_SKIP_PROMPT') != "1":
    log.p("Enter Edison host (Default: {0})".format(edison['host']))
    edison['host'] = input() or edison['host']
else:
    log.i("Edison host is", edison['host'])


if os.environ.get('EDISON_CLI_SKIP_PROMPT') != "1":
    log.p("Enter Edison password {0}".format(
        "(**********)" if edison['password'] else ""
    ))
    edison['password'] = getpass.getpass(prompt="") or edison['password']
else:
    log.i("Edison password is set")

ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.load_host_keys(os.path.expanduser(os.path.join("~", ".ssh", "known_hosts")))
ssh.connect(edison['host'], username="edison", password=edison['password'])

if "push" or "all" in commands:
    log.i("Pushing \"imc-server\" to Edison")

    sftp = ssh.open_sftp()

    for (dir_path, dir_names, filenames) in os.walk("imc-server"):
        for filename in filenames:
            local_path = os.path.join(dir_path, filename)
            server_path = os.path.join("/home/edison", local_path)

            sftp.put(local_path, server_path)

    sftp.close()

    log.i("Push success")

if "compile" or "all" in commands:
    stdin, stdout, stderr = ssh.exec_command("mkdir -p ~/imc-server/build && cd ~/imc-server/build && cmake .. && make")
    log.d("Compile output")
    log.stdout("    ", stdout)

if "run" or "all" in commands:
    stdin, stdout, stderr = ssh.exec_command("cd /home/edison/imc-server/build && ./imc-server")
    log.d("Run output")
    log.stdout("    ", stdout)

ssh.close()