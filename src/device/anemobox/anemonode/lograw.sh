#!/bin/bash

LOGPATH=/media/sdcard/n2k_raw/

mkdir -p "${LOGPATH}"

# assumes system clock is available. True for RTC enabled system only.
TODAY=$(date +"%Y-%m-%d")
BOOT=$(cat /home/anemobox/bootcount)

/anemonode/bin/candump -L can0 | split -l 40000 --filter='gzip -6 > $FILE.gz' - "${LOGPATH}/can0-${BOOT}-${TODAY}-"
