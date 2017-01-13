# This script should be run on the anemobox to test the synchronization
set -e
bash /root/disable_watchdog.sh
rm -rf /media/sdcard/logs/*
cp /usr/bin/dpkg /media/sdcard/logs/dpkg.log
rm -rf /media/sdcard/mail2
cd /anemonode
if sh sync_check.sh; then
  echo "sync_check should *not* succeed, something is wrong!"
  exit 1
fi
echo "############## perform_sync_test_anemobox.sh: Launch the service!"
./run.sh
