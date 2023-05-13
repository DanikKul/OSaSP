#!/bin/bash

PREV_GENFILE_BUILD="genfile"
GENFILE_EXEC="./genfile"
PATH_TO_FILE=""

function echo_ok {
  echo -e "\033[32m$1\033[0m"
}

function echo_info {
  echo -e "\033[95m$1\033[0m"
}

function echo_warn {
  echo -e "\033[33m$1\033[0m"
}

function echo_err {
  echo -e "\033[91m$1\033[0m"
}

function check_build {
  if test -f "$1"; then
    echo_ok "Previous build files found"
    make clean
  else
    echo_warn "Previous build not found"
  fi
}

function check_and_run_exec {
  if test -f "$1"; then
    echo_ok "Executable file '$1' found"
    echo_ok "Running..."
    $1 $2 $3
  else
    echo_err "Executable file '$1' not found"
    exit 1
  fi
}

cd ../genfile || exit
echo_info "\n[GENFILE]"
echo_info "\nProject 'COMPETITION' files"
ls
echo_info "\nCleaning previous build"
check_build $PREV_GENFILE_BUILD
echo_info "\nBuilding project"
make
echo_info "\nEnter path to file"
read -r filename
echo_info "\nEnter size"
read -r size
echo_info "\nStarting './genfile'"
rm "$filename"
check_and_run_exec "$GENFILE_EXEC" "$filename" "$size"
echo_info "\nFull path to file"
realpath "$filename"
echo_info "\n"