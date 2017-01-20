# This script should be run on the anemobox to test the synchronization
#!/bin/bash
set -e
/anemonode/synctest/sync_init.sh 1
cp /usr/bin/dpkg /media/sdcard/logs/dpkg.log
echo "############## perform_sync_test_anemobox.sh: Launch the service!"
/anemonode/run.sh
