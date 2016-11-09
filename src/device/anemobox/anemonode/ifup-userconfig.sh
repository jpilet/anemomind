#!/bin/bash

statefile=/tmp/network-state

PATH=/anemonode/bin:/sbin/:/bin:/usr/bin:/usr/sbin

function wpaIsConnected {
  ( wpa_cli -i wlan0 status | grep -q wpa_state=COMPLETED ) \
  && ( ifconfig wlan0 | grep -q "inet addr" )
}

function hostapdRunning {
  systemctl -q is-active hostapd \
    && ifconfig wlan0 | grep -q "inet addr:192.168.2.1"
}

function hostapd {
  if hostapdRunning ; then
    setState configured-ap
  else
    setState configuring-ap
    ifdown wlan0
    /etc/init.d/hostapd start
    setState configured-ap
  fi
}

function connectToNetwork {
  setState configuring-client
  systemctl stop hostapd
  ifdown wlan0
  rmmod bcm4334x
  modprobe bcm4334x
  if ifup wlan0=userconfigured ; then
    setState configured-client
  else
    hostapd
  fi
}

function setState {
  STATE=$1
  echo ${STATE} > ${statefile}
  echo "State changed to ${STATE}"
}

if [ -f ${statefile} ] ; then
  STATE=$(cat ${statefile})
else
  setState not-configured
fi

# Possible states:
# not-configured
# configuring-ap
# configured-ap
# configuring-client
# configured-client

echo "Network state: $STATE"

case "${STATE}" in
  not-configured)
    if [ -f /etc/network/interfaces.d/userconfigured ] ; then
      # the system is supposed to connect to a remote AP
      connectToNetwork
    else
      # the system is supposed to be start in AP mode
      hostapd 
    fi
    ;;
  configuring-ap)
    ;;
  configured-ap)
    ;;
  configuring-client)
    ;;
  configured-client)
    if ! wpaIsConnected ; then
      hostapd
    fi 
    ;;
esac



