#!/bin/bash
echo "Fetching box id..."
ssh box "ifconfig wlan0 | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}' | sed 's/[:\s]//g'" > boxid.txt
echo "The new box id is $(cat boxid.txt)"
