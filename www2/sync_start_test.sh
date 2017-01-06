set -e # Stop on first error

killall mongod || true
killall grunt || true
killall node || true
www2_dir=$(pwd)
cd /tmp && rm -rf endpoints && rm synctest_message.txt || true
cd "$www2_dir" 
cd uploads/anemologs && rm -rf *
cd "$www2_dir" 
echo "Launching the server!"
cd "$www2_dir"
if sh sync_check.sh; then
  echo "The sync_check.sh should not pass, something is wrong!"
  exit 1
fi
grunt serve:dev &

for i in `seq 1 8`;
do
    sleep 1s
    echo "... waiting a bit to let the server start before we continue."
done    

echo "This data should go on the box!" > /tmp/synctest_message.txt 
cd utilities 
node SendBoatData 57f678e612063872e749d481 /tmp/synctest_message.txt /media/sdcard/logs/synctest_message_dst.txt 
sleep 4s 
echo "Seems like things went well so far, now apply the AnemomindApp fixes and synchronize the app with the box and the server."
