#!/bin/bash

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

cd ../client || exit
echo_info "\n[CLIENT]"
echo_info "\nProject 'client' files"
ls
echo_info "\nEnter ip (type 'localhost' for 127.0.0.1)"
read -r address
echo_info "\nEnter port"
read -r port
check_and_run_exec "./client" "$address" "$port"