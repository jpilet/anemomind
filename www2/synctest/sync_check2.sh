#!/bin/bash
set -e
truth="d7b92cb70c333ff82616ff660799a078"
result=$(md5 -q ${WWW2_DIR}/uploads/anemologs/boat57f678e612063872e749d481/${1} || true)
truth="${2}"
if [ "$result" = "$truth" ]; then
  echo "PASSED :-)"
  exit 0
else
  echo "Failed, got '$result' but expected '$truth'"
  exit 1
fi
