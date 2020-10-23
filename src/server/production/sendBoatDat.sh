#!/bin/bash

export NODE_ENV="production"
export MONGOLAB_URI="${MONGO_URL}"
LOG_DIR="/db/uploads/anemologs"

processBoat() {
    local boatdat="$(echo "$1" | sed 's/-ready-to-send//')"
    local processed="$(echo "${boatdat}" | sed 's#/boat.dat$##')"
    local boatid=$(echo "${processed}" | sed 's#.*boat\(.*\)#\1#')

    #echo "boatdat: ${boatdat}"
    #echo "processed: ${processed}"
    #echo "boatid: ${boatid}"

    #echo node /app/utilities/SendBoatData.js \
    #      "${boatid}" "${boatdat}" /home/anemobox/boat.dat

    if node /app/utilities/SendBoatData.js "${boatid}" "${boatdat}" /home/anemobox/boat.dat ; then
      #echo "${boatdat} sent."
      rm "$1"
    else
      echo "Failed to send ${boatdat}".
    fi
}


for readyToSend in "${LOG_DIR}"/*/boat.dat-ready-to-send ; do
    [ -f "${readyToSend}" ] && processBoat "${readyToSend}"
done
