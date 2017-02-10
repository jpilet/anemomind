#!/bin/bash
# Accepts a single argument, the box id:
# The box id are the concatenation of the hexadecimal digits
# printed on the box label.
set -e
./sync_init.sh
sleep 8s
WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
node ${WWW2_DIR}/utilities/SendBoatData ${1} ${WWW2_DIR}/synctest/mock_boat.dat /home/anemobox/boat.dat 
echo "####### So far so good"
$(node ${WWW2_DIR}/utilities/RunRemoteScript.js ${1} ${WWW2_DIR}/synctest/ping_script.sh | grep "node" > /tmp/calltmp2.txt)
echo "######### Posted mock boat.dat and script that will check its arrival"
