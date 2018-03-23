#!/bin/bash
echo "When running this script, make sure that the box is either on your local network or you are connected to the local network of the box."
echo "If 'ssh box' does not work, then this script won't work."
echo "Fetching box id..."
ssh box "ifconfig wlan0 | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}' | sed 's/[:\s]//g'" > boxid.txt
echo "The new box id is $(cat boxid.txt)"
