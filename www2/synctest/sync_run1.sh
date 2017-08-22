#!/bin/bash
set -e
./sync_init.sh 1
sleep 8s
WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
boatId=$(${WWW2_DIR}/synctest/get_devbox_boatid.sh)
echo "This data should go on the box!" > /tmp/synctest_message.txt 
echo "POSTING TO BOAT ${boatId}"
node "${WWW2_DIR}/utilities/SendBoatData" ${boatId} /tmp/synctest_message.txt /media/sdcard/logs/synctest_message_dst.txt
sleep 4s 
echo "Seems like things went well so far, now apply the AnemomindApp fixes and synchronize the app with the box and the server."
