#!/bin/bash

PREV_GENFILE_BUILD="genfile"
GENFILE_EXEC="./genfile"

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
    $1
  else
    echo_err "Executable file '$1' not found"
    exit 1
  fi
}

cd ../genfile
echo_info "\n[GENFILE]"
echo_info "\nProject 'genfile' files"
ls
echo_info "\nCleaning previous build"
check_build $PREV_GENFILE_BUILD
echo_info "\nBuilding project"
make
echo_info "\nStarting './genfile'"
check_and_run_exec "$GENFILE_EXEC"