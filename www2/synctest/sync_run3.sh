#!/bin/bash
set -e
./sync_init.sh 3
./wait_a_moment.sh
./wait_a_moment.sh
node ../utilities/RunRemoteScript.js 57f678e612063872e749d481 sample_script.sh | grep "node" > /tmp/calltmp.txt
echo "######################################################################################################################################################################################################################################################################################################################################################### Posted script"

