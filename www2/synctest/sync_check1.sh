#!/bin/bash
set -e
WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
boatId=$(${WWW2_DIR}/synctest/get_devbox_boatid.sh)
truth="d7b92cb70c333ff82616ff660799a078"
result=$(md5 -q ../uploads/anemologs/boat${boatId}/dpkg.log || true)
if [ "$result" = "$truth" ]; then
  echo "PASSED :-)"
  exit 0
else
  echo "Failed, got '$result' but expected '$truth'"
  exit 1
fi
