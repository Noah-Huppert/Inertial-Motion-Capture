import os, argparse, sys, logging, subprocess

from colorlog import ColoredFormatter

# Setup logging
logging_formatter = ColoredFormatter(
        "%(log_color)s%(levelname)-8s%(reset)s %(blue)s%(message)s",
        datefmt=None,
        reset=True,
        log_colors={
                'DEBUG':    'cyan',
                'INFO':     'green',
                'WARNING':  'yellow',
                'ERROR':    'red',
                'CRITICAL': 'red,bg_white',
        },
        secondary_log_colors={},
        style='%'
)

logger = logging.getLogger("Edison CLI")
handler = logging.StreamHandler()
handler.setFormatter(logging_formatter)
logger.addHandler(handler)
logger.setLevel(logging.DEBUG)

# Setup CLI
parser = argparse.ArgumentParser(description="CLI for edison")
subparsers = parser.add_subparsers(help='sub-command help')

parser.add_argument("--host",
                    help="Intel Edison host")

parser.add_argument("--user",
                    help="Intel Edison user",
                    default="root")

parser.add_argument("--password",
                    help="Intel Edison password")

parser.add_argument("--remote-path",
                    help="The path imc-server will be pushed to on the Intel Edison",
                    default="~/imc-server")

subparsers.add_parser("push-server",
                    help="Pushes the imc server to the Edison")

subparsers.add_parser("ssh",
                    help="Opens up a ssh connection to the Intel Edison")

args = parser.parse_args()

# Check for dependencies
try:
    sshpass_check = subprocess.check_output(["which", "sshpass"])
except:
    logger.critical("\"sshpass\" must be installed")
    sys.exit()

# Checks to see if the specified command line argument is present
#
# @param arg The key of the command line argument
# @returns True if present, False if not
def check_arg(arg):
    if arg not in args or getattr(args, arg) == None:
        return False
    else:
        return True

# Checks to see if each of the provided command line arguments are present
#
# @param needed_args An array of required command line arguments
# @param action The name of the action that requires these arguments, used in
#   notifying the user of missing arguments
def check_args(needed_args, action):
    not_present_args = []

    for arg in needed_args:
        if not check_arg(arg):
            not_present_args.append(arg)

    if len(not_present_args) > 0:
        not_present_args_string = ""

        i = 0
        for arg in not_present_args:
            not_present_args_string += "\"{0}\"".format(arg)

            if i == len(not_present_args) - 2:# Second to last
                not_present_args_string += " and "
            elif i != len(not_present_args) - 1:# Not last
                not_present_args_string += ", "

            i += 1

        logger.critical("The argument{0} {1} {2} required for the \"{3}\" command".format(
            "s" if len(not_present_args) > 1 else "",
            not_present_args_string,
            "are" if len(not_present_args) > 1 else "is",
            action
        ))
        sys.exit()

if check_arg("push-server"):
    check_args(["host", "user", "password", "remote_path"], "push-server")

    os.system(
        "sshpass -p \"{0}\" rsync -avz --progress -e ssh {1}@{2}:{3} ./imc-server".format(
            args.password,
            args.user,
            args.host,
            args.remote_path
        )
    )

if check_arg("ssh"):
    check_args(["host", "user", "password"], "ssh")

    os.system(
        "sshpass -p {0} ssh {1}@{2}".format(
            args.password,
            args.user,
            args.host
        )
    )
