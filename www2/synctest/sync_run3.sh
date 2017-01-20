set -e
bash sync_init.sh
node ../utilities/RunRemoteScript.js 57f678e612063872e749d481 sample_script.sh  > /tmp/command_to_view.txt
echo "##### Script was posted"
