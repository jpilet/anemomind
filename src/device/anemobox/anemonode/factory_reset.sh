#!/bin/bash
## TODO: Should we just do 'rm -rf /media/sdcard/*' or would it be too much?
##       Should we just do 'rm -rf /home/anemobox/*' or would it be too much?
rm -rf /media/sdcard/mail2/*
rm -rf /media/sdcard/logs/*
rm /home/anemobox/config.json 
rm /home/anemobox/boat.dat
rm /home/anemobox/imu.cal
bootcount=$(</home/anemobox/bootcount)
if ! [[ $bootcount =~ ^[[:space:]]*[[:digit:]]+[[:space:]]*$ ]] ; then
  echo "WARNING: Boot count '$bootcount' was not a number, reset it to 0."
  echo "0" > /home/anemobox/bootcount
fi
echo "Successfully factory reset"
