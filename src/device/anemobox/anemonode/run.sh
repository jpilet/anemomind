#!/bin/bash
cd "$( dirname "${BASH_SOURCE[0]}" )"

RTC=/bin/rtc
[ -e "${RTC}" ] && "${RTC}" --load

set -e
export ANEMOBOX_CONFIG_PATH=/home/anemobox
export BLENO_DEVICE_NAME="Anemobox"
node main.js
