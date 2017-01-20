# This script should be run on the anemobox to test the synchronization
set -e
bash /anemonode/synctest/sync_init.sh 2
cd /anemonode
git checkout demo -- anemobox_playnmea replay.txt
./anemobox_playnmea replay.txt > /dev/ttyMFD1 &
first_post=$(date -d "+5 minutes")
echo "### Launch the service!"
echo "### In 5 minutes, $first_post, you can expect a message"
echo "### being posted saying 'Posted this log file'"
bash /anemonode/run.sh
