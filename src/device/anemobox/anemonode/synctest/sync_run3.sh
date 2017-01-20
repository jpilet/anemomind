# This script should be run on the anemobox to test the synchronization
set -e
bash /anemonode/synctest/sync_init.sh 3
echo "### Now you only have to wait for the script to arrive here and execute."
bash /anemonode/run.sh
