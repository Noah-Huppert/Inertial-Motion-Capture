import os, sys, subprocess, getpass, paramiko, re

from collections import OrderedDict

# Log helpers
class colors:
    RED     = "\033[31m"
    GREEN   = "\033[32m"
    YELLOW  = "\033[33m"
    BLUE    = "\033[34m"
    PURPLE  = "\033[35m"
    CYAN    = "\033[36m"
    GRAY    = "\033[90m"

    ENDC    = "\033[0m"

TAG_PROMPT = colors.GREEN   + "[ PROMPT ]" + colors.ENDC
TAG_INFO   = colors.YELLOW  + "[ INFO   ]" + colors.ENDC
TAG_DEBUG  = colors.BLUE    + "[ DEBUG  ]" + colors.ENDC
TAG_ERROR  = colors.RED     + "[ ERROR  ]" + colors.ENDC

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

    def std(self, prefix, std, print_out = True):
        out = ""
        for line in iter(lambda: std.readline(2048), ""):
            line = "{0}{1}".format(prefix, line)

            out += line

            if print_out:
                print(line, end="")

        return out

    def stds(self, prefix, stdout, stderr):
        self.std(prefix, stderr)
        self.std(prefix, stdout)

log = Log()

# Get Edison credentials
edison = {}
edison["host"] = os.environ.get("EDISON_HOST") or "192.168.2.15"
edison["user"] = os.environ.get("EDISON_USER") or "root"
edison["password"] = os.environ.get("EDISON_PASSWORD")

skip_cli_prompt = os.environ.get("EDISON_CLI_SKIP_PROMPT") == "1"

# Prompt helpers
def should_prompt_for(env_key):
    if skip_cli_prompt:
        return False
    elif os.environ.get(env_key) == None:
        return True

    return False

def prompt_for(env_key, key, display_value = True):
    if should_prompt_for(env_key):
        log.p("Enter Edison {0}".format(key))
        edison[key] = input() or edison[key]
    elif display_value == False:
        log.p("Edison {0} is set".format(key))
    else:
        log.p("Edison {0} is".format(key), edison[key])

def traverse_commands(commands, command_defs):
    for command_def in command_defs:
        if type(command_defs[command_def]) == OrderedDict:
            if command_def in commands:
                for deep_command in command_defs[command_def]:
                    traverse_commands(deep_command, command_defs[command_def])
            else:
                traverse_commands(commands, command_defs[command_def])
        elif command_def in commands:
            command_defs[command_def]()


# Command helpers
prompted = False
def prompt_for_edison_info():
    global prompted
    if prompted == False:
        prompt_for("EDISON_HOST", "host")
        prompt_for("EDISON_USER", "user")
        prompt_for("EDISON_PASSWORD", "password", False)

        prompted = True

ssh = None
def setup_ssh_client():
    global ssh
    if ssh == None:
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.load_host_keys(os.path.expanduser(os.path.join("~", ".ssh", "known_hosts")))
        ssh.connect(edison["host"], username=edison["user"], password=edison["password"])

sshpass_checked = False
def check_sshpass():
    global sshpass_checked
    if sshpass_checked == False:
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

        sshpass_checked = False

def run_ssh_command(command):
    run_helpers("edison_info", "check_sshpass")

    return os.system("sshpass -p {0} ssh {1}@{2} \"{3}\"".format(edison["password"], edison["user"], edison["host"], command))

def run_helpers(*args):
    if "edison_info" in args:
        prompt_for_edison_info()

    if "ssh_client" in args:
        setup_ssh_client()

    if "check_sshpass":
        check_sshpass()

# Commands
def command_clean():
    run_helpers("edison_info", "ssh_client")

    log.i("Cleaning \"imc-server\"")

    run_ssh_command("rm -rf /home/edison/imc-server")

def command_push():
    run_helpers("edison_info", "ssh_client")

    log.i("Pushing \"imc-server\" to Edison")

    sftp = ssh.open_sftp()

    for (dir_path, dir_names, filenames) in os.walk("imc-server"):
        for filename in filenames:
            local_path = os.path.join(dir_path, filename)
            server_path = os.path.join("/home/edison", local_path)

            try:
                sftp.mkdir("/home/edison/{0}".format(dir_path))
            except:
                pass

            try:
                sftp.put(local_path, server_path)
            except FileNotFoundError:
                log.e("Failed to push \"{0}\", does not exist".format(local_path))

    sftp.close()

    log.i("Push success")

def command_compile():
    run_helpers("edison_info", "ssh_client")

    log.i("Compiling \"imc-server\"")

    compile_exit_code = run_ssh_command("mkdir -p /home/edison/imc-server/build && cd /home/edison/imc-server/build && cmake .. && make")

    if compile_exit_code != 0:
        log.e("Failed to compile \"imc-server\"")
        sys.exit(0)

def command_run():
    log.i("Running \"imc-server\"")

    run_ssh_command("cd /home/edison/imc-server/build && ./imc-server")

def command_kill():
    log.i("Killing \"imc-server\"")

    netstat_p = subprocess.Popen(
                            [
                            "sshpass",
                            "-p",
                            edison["password"],
                            "ssh",
                            "{0}@{1}".format(edison["user"], edison["host"]),
                            "netstat",
                            "-lpn"
                            ],
                         shell=False,
                         stdout=subprocess.PIPE)

    (netstat_out, netstat_err) = netstat_p.communicate()

    netstat_out = netstat_out.decode("utf-8")

    pid_exp = re.compile("(\d\d\d\d)\/imc-server")
    pid_match = re.search(pid_exp, netstat_out)

    try:
        pid = pid_match.group(1)

        run_ssh_command("kill -9 {0}".format(pid))
        log.i("\"imc-server\" killed successfully")
    except:
        log.i("\"imc-server\" not running")

def command_ssh():
    run_helpers("edison_info", "check_sshpass")

    log.i("Accessing Edison via SSH")

    run_ssh_command("")

def command_serial():
    log.i("Accessing Edison via serial")

    if not os.path.isfile("/dev/ttyUSB0"):
        log.e("Can not find Edison on \"/dev/ttyUSB0\"")
        return

    os.system("sudo screen /dev/ttyUSB0 115200")

# Get commands
commands = sys.argv
commands.pop(0)

command_defs = OrderedDict([
    ("clean", command_clean),
    ("build",
        OrderedDict([
            ("push", command_push),
            ("compile", command_compile),
            ("run", command_run),
            ("kill", command_kill)
        ])
    ),
    ("ssh", command_ssh),
    ("serial", command_serial)
])

traverse_commands(commands, command_defs)

if ssh != None:
    ssh.close()