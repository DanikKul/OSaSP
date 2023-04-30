#!/bin/bash

PREV_SORT_BUILD="sort_index"
SORT_INDEX_EXEC="./sort_index"

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

echo_info "\n[SORT_INDEX]"
cd ../sort_index
echo_info "\nProject 'sort_index' files"
ls
echo_info "\nCleaning previous build"
check_build "$PREV_SORT_BUILD"
echo_info "\nBuilding project"
make
echo_info "\nStarting './sort_index'"
check_and_run_exec "$SORT_INDEX_EXEC"