#!/bin/bash
## Common initialization for different tests. Called by sync_run*.sh
## Provide as argument the index of the test you are running. For instance,
## if you run 'sync_run2.sh' with a corresponding check 'sync_check2.sh', then
## pass '2' as argument to this program.
set -e
[ -e "/anemonode/synctest/sync_check${1}.sh" ] || (echo "please pass a valid test number as argument" ; exit 1 )
/root/disable_watchdog.sh
rm -rf /media/sdcard/logs/*
rm -rf /media/sdcard/mail2
export NO_WATCHDOG=1
if "/anemonode/synctest/sync_check${1}.sh"; then
  echo "sync_check should *not* succeed, something is wrong!"
  exit 1
fi
cd /anemonode
echo "Initialized with index ${1}"
