#!/bin/bash
set -e
result=$(cat /media/sdcard/logs/message.txt || true)
if [ "$result" = "This was written by a script" ]; then
  echo "Passed :-)"
  exit 0
else
  echo "Failed :-("
  exit 1
fi
