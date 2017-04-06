#!/bin/bash
set -e
result=$(ls /media/sdcard/logs/sentlogs/ | grep ".log" || true)
if [ "$result" = "" ]; then
  echo "Failed :-( (no posted log files)"
  exit 1
else
  echo "Passed :-)"
  echo "This is the file posted: $result"
  echo ""
  code=($(md5sum "/media/sdcard/logs/sentlogs/$result"))
  echo "Once you have synchronized with the server,"
  echo "please call"
  echo ""
  echo "  ./sync_check2.sh $result $code"
  echo ""
  exit 0
fi
