#!/bin/bash
mkdir -p /home/anemobox
cd "$( dirname "${BASH_SOURCE[0]}" )"

RTC=/bin/rtc
[ -e "${RTC}" ] && "${RTC}" --load

./format.sh

ps aux | grep -v grep | grep -q jacd || ./start_n2k.sh

set -e
export ANEMOBOX_CONFIG_PATH=/home/anemobox
export BLENO_DEVICE_NAME="Anemobox"
node main.js
