# This script should be run on the anemobox to test the synchronization
set -e
bash /root/disable_watchdog.sh
cd /media/sdcard/logs && rm -rf *
cp /usr/bin/dpkg dpkg.log
cd /media/sdcard/mail2
rm -rf *
cd /anemonode
if sh sync_check.sh; then
  echo "sync_check should *not* succeed, something is wrong!"
  exit 1
fi
echo "############## perform_sync_test_anemobox.sh: Launch the service!"
./run.sh > synctest.log
