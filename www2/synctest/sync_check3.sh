#!/bin/bash
set -e
cd ../utilities
truth="Pine needle tea"
result=$(node ViewRemoteScript.js $SCRIPTCODE | grep "Pine" || true)
if [ "$result" = "$truth" ]; then
  echo "PASSED :-)"
  exit 0
else
  echo "Failed, got '$result' but expected '$truth'"
  exit 1
fi
