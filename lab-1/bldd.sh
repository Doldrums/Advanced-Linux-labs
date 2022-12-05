#!/bin/bash

# Fail if there are any unset variables and whenever a command returns a
# non-zero exit code.
# set -eu

# # Black        0;30     Dark Gray     1;30
# # Red          0;31     Light Red     1;31
# # Green        0;32     Light Green   1;32
# # Brown/Orange 0;33     Yellow        1;33
# # Blue         0;34     Light Blue    1;34
# # Purple       0;35     Light Purple  1;35
# # Cyan         0;36     Light Cyan    1;36
# # Light Gray   0;37     White         1;37

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color
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
        $program [FLAGS] [OPTIONS] [ARG]
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

bldd(){
    echo "bldd started with arguments: so=$1 directory=$2 verbose=$3 format=$4"
    echo "Results for $1" > result.$4
    if [ "$2" != "default" ]; then
        for file in $(find $2 -type f); do
            if ldd $file 2> /dev/null | grep $1 &> /dev/null; then
                echo "$file" >> result.$4
            fi
        done
    else
        array=(
            "$HOME/bin/" #Local binaries
            "$HOME/etc/" #Host-specific system configuration for local binaries
            "$HOME/games/" #Local game binaries
            "$HOME/include/" #Local C header files
            "$HOME/lib/" #Local libraries
            "$HOME/lib64/" #Local 64-bit libraries
            "$HOME/man/" #Local online manuals
            "$HOME/sbin/" #Local system binaries
            "$HOME/share/" #Local architecture-independent hierarchy
            "$HOME/src/" #Local source code
        )
        for i in "${array[@]}"; do 
            if [ -d "$i" ]; then
                if [ $3==true ]; then 
                    echo -e "Proceeding ${i}..." 
                fi
                for file in $(find $i -type f ); do
                    if ldd $file 2> /dev/null | grep $1 &> /dev/null; then
                        echo "$file" >> result.$4
                    fi
                done
            else
                if [ $3==true ]; then 
                    echo -e "Directory ${i} will not be proceed." 
                fi
            fi
        done
    fi
}

so="libc"
directory="default"
format="txt"
verbose=false

while getopts ":hvVr:d:l:" flag; do
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
            verbose=true
            ;;
        r) 
            format=${OPTARG}
            ;;
        d) 
            directory=${OPTARG}
            ;;
        l)
            so=${OPTARG}
            ;;
        \?)
            exit_with "Invalid option" 1
            ;;
    esac
done

echo -e "${CYAN}Backward LDD.${NC}\n"   
echo -e "${CYAN}Looking for executable objects for $so Shared library in $directory directory..${NC}\n"
bldd "$so" "$directory" "$verbose" "$format"
echo -e "${CYAN}Congrats! All done. Result saved to $pwd/result.txt.${NC}\n"
exit 0