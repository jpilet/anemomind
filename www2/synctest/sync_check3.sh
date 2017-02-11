#!/bin/bash
set -e
WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
truth="Pine needle tea"
to_execute=$(cat /tmp/calltmp.txt)
echo "What to execute: $to_execute"
if (cd "${WWW2_DIR}/utilities" ; bash /tmp/calltmp.txt ) | grep -q "${truth}" ; then
  echo "PASSED :-)"
  exit 0
else
  echo "Failed, got '$result' but expected '$truth'"
  exit 1
fi
