#!/bin/bash
set -e
truth="This data should go on the box!"
result=$(cat /media/sdcard/logs/synctest_message_dst.txt)
if [ "$result" = "$truth" ]; then
  echo "PASSED :-)"
  exit 0
else
  echo "Failed, got '$result' but expected '$truth'"
  exit 1
fi
