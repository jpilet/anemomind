#!/bin/bash
set -e # Stop on first error
cd ..
killall mongod || true
killall grunt || true
killall node || true
export WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "WWW2_DIR = $WWW2_DIR"
rm -rf /tmp/endpoints || true
rm -r /tmp/synctest_message.txt || true
rm -rf "${WWW2_DIR}/uploads/anemologs/boat57f678e612063872e749d481"
echo "Launching the server!"
cd synctest
if eval "./sync_check${1}.sh"; then
  echo "The sync_check.sh should not pass, something is wrong!"
  exit 1
fi
grunt serve:dev &
echo "### The server was started"
