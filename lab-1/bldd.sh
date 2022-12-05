#!/bin/bash

# Fail if there are any unset variables and whenever a command returns a
# non-zero exit code.
set -eu

version='0.0.1'
# The author of this program
author='Arina Cheverda BS20-CS-01'
# The short version of the program name which is used in logging output
program=$(basename "$0")

# **Internal** Exit the program with an error message and a status code.
#
# ```sh
# exit_with "Something bad went down" 55
# ```
exit_with() {
  if [ "${HAB_NOCOLORING:-}" = "true" ]; then
    echo "ERROR: $1"
  else
    case "${TERM:-}" in
      *term | xterm-* | rxvt | screen | screen-*)
        printf -- "\033[1;31mERROR: \033[1;37m%s\033[0m\n" "$1"
        ;;
      *)
        echo "ERROR: $1"
        ;;
    esac
  fi
  exit "$2"
}

# **Internal**  Prints help and usage information.
usage() {
  echo "$program $version
$author
Backward LDD - prints the EXECUTABLE objects required by each
       program or shared object specified on the command line.  An
       example of its use and output is the following:
USAGE:
        $program [FLAGS] [OPTIONS] <SUBCOMMAND> [ARG ..]
COMMON FLAGS:
    -h  Prints Usage information.
    -v  Prints more verbose output.
    -V  Prints version information.
COMMON OPTIONS:
    -d <SCAN_DIR_PATH>    Sets the source scan directory path (default: /hab/cache/artifacts)
    -r <REPORT_FILE_TYPE> Sets Report file type (default: txt)
                          Valid types: [txt pdf]
EXAMPLES:
    # BLDD displays the location of the matching object and the (hexadecimal) address at which it is loaded
    $program -d /bin/ls
    # Verbosely displays the results
    $program -v -r /opt/slim
"
}

# The getopts function reads the flags in the input, and OPTARG refers to the corresponding values:
while getopts ":h:v:V:r:d:" flag; do
    case "${flag}" in
        h)
            usage
            exit 0
            ;;
        v)
            echo "$program $version"
            exit 0
            ;;
        V) 
            # чото видимо запускаем с логами
            exit 0
            ;;
        r) 
            format=${OPTARG}
            # чото видимо запускаем с форматом репорта
            exit 0
            ;;
        d) 
            directory=${OPTARG}
            # чото видимо запускаем
            exit 0
            ;;
        \?)
            exit_with "Invalid option" 1
            ;;
        *)
            usage
            ;;
    esac
done
