set -e
bash sync_init.sh
bash wait_a_moment.sh
echo "This data should go on the box!" > /tmp/synctest_message.txt 
node "../utilities/SendBoatData" 57f678e612063872e749d481 /tmp/synctest_message.txt /media/sdcard/logs/synctest_message_dst.txt 
sleep 4s 
echo "Seems like things went well so far, now apply the AnemomindApp fixes and synchronize the app with the box and the server."
