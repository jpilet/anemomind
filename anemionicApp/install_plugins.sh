#!/bin/sh
for i in com.evothings.ble com.ionic.keyboard org.apache.cordova.console org.apache.cordova.device; do
  cordova plugin rm $i 
  cordova plugin add $i
done
