#!/bin/bash
cd "$( dirname "${BASH_SOURCE[0]}" )"
set -e
export ANEMOBOX_CONFIG_PATH=/home/anemobox
export BLENO_DEVICE_NAME="Anemobox"
node main.js
