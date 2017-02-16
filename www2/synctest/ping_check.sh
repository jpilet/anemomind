#!/bin/bash
set -e
WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
truth="109482d245aa982cb550f7c5be67ffa3" ## md5 on mock_boat.dat
if (cd "${WWW2_DIR}/utilities" ; bash /tmp/calltmp2.txt ) | grep -q "${truth}" ; then
  echo "PASSED :-)"
  rm /tmp/calltmp2.txt
  exit 0
else
  echo "Failed :-("
  exit 1
fi
