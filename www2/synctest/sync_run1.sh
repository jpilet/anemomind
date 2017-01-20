set -e
bash sync_init.sh
for i in `seq 1 8`;
do
    sleep 1s
    echo "... waiting a bit to let the server start before we continue."
done
echo "This data should go on the box!" > /tmp/synctest_message.txt 
node "../utilities/SendBoatData" 57f678e612063872e749d481 /tmp/synctest_message.txt /media/sdcard/logs/synctest_message_dst.txt 
sleep 4s 
echo "Seems like things went well so far, now apply the AnemomindApp fixes and synchronize the app with the box and the server."
