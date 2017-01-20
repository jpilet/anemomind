#!/bin/bash
set -e # Stop on first error
cd ..
killall mongod || true
killall grunt || true
killall node || true
WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
LOGFILE="${WWW2_DIR}/uploads/anemologs/boat57f678e612063872e749d481/dpkg.log"
rm -rf /tmp/endpoints || true
rm -r /tmp/synctest_message.txt || true
rm -rf "${LOGFILE}"
echo "Launching the server!"
cd synctest
if eval "./sync_check${1}.sh"; then
  echo "The sync_check.sh should not pass, something is wrong!"
  exit 1
fi
grunt serve:dev &
echo "### The server was started"
