#!/bin/bash

set -e

HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BIN="${HERE}/bin"
# See https://github.com/kurt-vd/test-can-j1939
# and
# http://elinux.org/J1939

# $ADDR corresponds to PNG 60928 ISO Address Claim
#
# combine 21 bits from MAC address and a hand picked manufacturer code
# manufacturer code: 925 (0x39d) * 2 = 73a

IP=${BIN}/ip

# do we need to configure can0?
if ! (ifconfig | grep -q can0) ; then
  $IP link set can0 type can bitrate 250000 triple-sampling on restart-ms 100
  while ! $IP link set can0 up; do sleep 1; done
fi

