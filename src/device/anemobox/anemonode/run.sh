#!/bin/bash
mkdir -p /home/anemobox
cd "$( dirname "${BASH_SOURCE[0]}" )"

if [ -f ~/.ssh/id_rsa ]; then
  true
else
  echo "installing SSH Key"
  mkdir -p ~/.ssh
  cp id_rsa* ~/.ssh
  chmod 600 ~/.ssh/id_rsa
  cat known_hosts >> ~/.ssh/known_hosts
fi

./format.sh

ps aux | grep -v grep | grep -q jacd || ./start_n2k.sh

set -e
export ANEMOBOX_CONFIG_PATH=/home/anemobox
export BLENO_DEVICE_NAME="Anemobox"
node main.js
