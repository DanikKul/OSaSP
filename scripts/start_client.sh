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


cd ../client || exit
echo_info "\n[CLIENT]"
echo_info "\nProject 'client' files"
ls
echo_info "\nTo manipulate IP and address change config/config.json"
python3 client.py