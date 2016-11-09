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

MAC=$(ifconfig wlan0 | grep HWaddr | sed 's/.*HWaddr /0x/' | sed 's/://g')
UNIQUE_ID=$(node -e "console.log(((${MAC} & 0x1fffff) | 0x73a00000).toString(16))" )
ADDR="0xc0288c00${UNIQUE_ID}"

modprobe can-j1939

IP=${BIN}/ip

# do we need to configure can0?
if ! (ifconfig | grep -q can0) ; then
  $IP link set can0 type can bitrate 250000 triple-sampling on restart-ms 100
  while ! $IP link set can0 up; do sleep 1; done
  $IP link set can0 j1939 on
  $IP addr add j1939 name $ADDR dev can0
  echo "Interface can0 configured with addr: $ADDR"
fi

# do we need to run jacd?
if ! (ps aux | grep -v grep | grep jacd -q) ; then
  echo "starting address claim daemon with addr: $ADDR"
  ${BIN}/jacd $ADDR &
fi

