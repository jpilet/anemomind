#!/bin/bash
set -e # Stop on first error
killall mongod || true
killall grunt || true
killall node || true
WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
boatId=$(${WWW2_DIR}/synctest/get_devbox_boatid.sh)
echo "######################### WWW2_DIR = $WWW2_DIR"
export LOGFILE="${WWW2_DIR}/uploads/anemologs/boat${boatId}/dpkg.log"
rm -rf /tmp/endpoints || true
rm -r /tmp/synctest_message.txt || true
rm -rf "${LOGFILE}"
echo "Launching the server!"
if "${WWW2_DIR}/synctest/sync_check${1}.sh" ; then
  echo "The sync_check.sh should not pass, something is wrong!"
  exit 1
fi
(cd "${WWW2_DIR}" ; grunt serve:dev) &
echo "### The server was started"
