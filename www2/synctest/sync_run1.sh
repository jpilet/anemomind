set -e # Stop on first error
cd ..
killall mongod || true
killall grunt || true
killall node || true
www2_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "www2_dir = $www2_dir"
rm -rf /tmp/endpoints || true
rm -r /tmp/synctest_message.txt || true
rm -rf "${www2_dir}/uploads/anemologs/boat57f678e612063872e749d481"
echo "Launching the server!"
cd synctest
if eval "bash sync_check${1}.sh"; then
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
node "${www2_dir}/utilities/SendBoatData" 57f678e612063872e749d481 /tmp/synctest_message.txt /media/sdcard/logs/synctest_message_dst.txt 
sleep 4s 
echo "Seems like things went well so far, now apply the AnemomindApp fixes and synchronize the app with the box and the server."
