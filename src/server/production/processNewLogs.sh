#!/bin/bash
#set -e

# This script can be run from crontab with:
#   */5 *  *   *   *   ps aux | grep -v grep | grep processNewLogs || /home/anemomind/bin/processNewLogs.sh

export BIN="/home/anemomind/bin"
export LOG_DIR="/db/anemologs/anemologs"
export PROCESSED_DIR="/home/anemomind/processed"
export SRC_ROOT="/home/jpilet/anemomind"
export BUILD_ROOT=${SRC_ROOT}/build

ulimit -c unlimited

log() {
  echo $(date +"%Y-%m-%d %T") $*
}

loginfo() {
  if [[ "${_V}" -eq 1 ]]; then
    log $*
  fi
}

if [[ "${_V}" -eq 1 ]]; then
export NOINFO=""
else
export NOINFO="--noinfo"
fi

safeRun() {
  loginfo "Running: " $*
    $*
#timeout 24h $*
}

processBoat() {

  local boatdir=$1
  local boat=$(basename "${boatdir}")
  local boatid=$(echo "${boat}" | sed 's/boat//')
  local boatprocessdir="${PROCESSED_DIR}/${boat}"
  mkdir -p "${boatprocessdir}"
  local lastprocess="${boatprocessdir}/lastprocess.md5"

  local new_md5=$(ls -lR "${boatdir}" | md5sum)
 
  if [ -e "${lastprocess}" ] && echo "${new_md5}" | diff -q "${lastprocess}" - ; then
    # checksum OK, nothing to do.
    loginfo "Skipping boat: ${boat}"
    true
  else
    # checksum not present or not up to date: need recompute.

    log "Recomputing for boat: ${boat}"

    local boatdat="${boatprocessdir}/processed/boat.dat"
    [ -f "${boatdat}" ] && rm -f "${boatdat}"

    # log converted to .TXT are not needed anymore, processBoatLogs will read the
    # .log files directly.
    rm -f "${boatprocessdir}/LOG.TXT" || true

    # processBoatLogs takes only 1 arg. Create a symlink to pass the source
    # directory.
    [ -L "${boatprocessdir}/logs" ] || ln -s "${boatdir}" "${boatprocessdir}/logs"

    local processed="${boatprocessdir}/processed"
    test -d "${processed}" || mkdir "${processed}"

    [ "${boatid}" == "5ca376093732e93fb87c8cf2" ] && GPS_FILTER="--no-gps-filter"
    
    if safeRun "${BUILD_ROOT}"/src/server/nautical/nautical_processBoatLogs \
        ${NOINFO} ${GPS_FILTER} \
        --dir "${boatprocessdir}" \
        --dst "${processed}" \
        --boatid "${boatid}" \
        --save-default-calib \
        -t --clean -c \
	--mongo-uri "mongodb://anemomindprod:$MONGO_PASSWORD@anemolab1,anemolab2,compute3,arbiter/anemomind?replicaSet=rs0" \
	--scale 20 ; then

      if [ -f "${boatdat}" ] ; then
        # If a boat.dat file has been generated, mail it to the anemobox.
        cat "${boatdat}" | ssh anemomind@anemolab1 NODE_ENV=production \
	  MONGOLAB_URI="mongodb://anemomindprod:$MONGO_PASSWORD@anemolab1,anemolab2,compute3,arbiter/anemomind?replicaSet=rs0" \
          node /home/jpilet/anemomind/www2/utilities/SendBoatData.js \
          "${boatid}" /dev/stdin /home/anemobox/boat.dat || true
          #node /home/xa4/anemomind/www2/utilities/SendBoatData.js \
      fi

      # Recompute worked. Update the checksum.
      echo "${new_md5}" > "${lastprocess}"

      # Update data associated with events
      safeRun mongo --quiet \
	      "mongodb://anemomindprod:$MONGO_PASSWORD@anemolab1,anemolab2,compute3,arbiter/anemomind?replicaSet=rs0" \
        --eval "boatid='${boatid}';onlynew=false;" \
        "${SRC_ROOT}"/src/server/production/extendEvents.js
    else
      log "processBoatLogs FAILED for ${boatprocessdir}. Skipping upload."
    fi
  fi
}

# Inspired by https://www.gnu.org/software/parallel/parallel_tutorial.html
# Exporting the function allows "parallel" to call it.
export -f processBoat
export -f log
export -f loginfo
export -f safeRun

export SHELL=/bin/bash
parallel  -j 2 processBoat ::: "${LOG_DIR}/"*

safeRun "${BIN}"/uploadVmgTable.sh
