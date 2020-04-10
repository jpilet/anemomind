#!/bin/bash

export BIN="/anemomind/bin"
export LOG_DIR="/db/anemologs/anemologs"
export PROCESSED_DIR="/db/processed"

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

    
    if safeRun "${BIN}"/nautical_processBoatLogs \
        ${NOINFO} \
        --dir "${boatprocessdir}" \
        --dst "${processed}" \
        --boatid "${boatid}" \
        --save-default-calib \
        -t --clean -c \
        --mongo-uri ${MONGO_URL} \
	--scale 20 ; then

    
    #  if [ -f "${boatdat}" ] ; then
    #    # If a boat.dat file has been generated, mail it to the anemobox.
    #    cat "${boatdat}" | ssh anemomind@anemolab1 NODE_ENV=production \
    #      # MONGOLAB_URI=mongodb://anemomindprod:${MONGO_PASSWORD}@anemolab1,anemolab2,arbiter/anemomind \
    #      ${MONGO_URL} \
    #      node /home/jpilet/anemomind/www2/utilities/SendBoatData.js \
    #      "${boatid}" /dev/stdin /home/anemobox/boat.dat || true
    #     #node /home/xa4/anemomind/www2/utilities/SendBoatData.js \
    #  fi

      # Recompute worked. Update the checksum.
      echo "${new_md5}" > "${lastprocess}"

    #   Update data associated with events
    #   safeRun mongo --quiet \
    #   -u anemomindprod -p "${MONGO_PASSWORD}" \
    #   anemolab1,anemolab2,arbiter/anemomind \
    #    ${MONGO_URL} \
    #    --eval "boatid='${boatid}';onlynew=false;" \
    #    "${BIN}"/extendEvents.js
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

# process each boat 
for boatDir in "${LOG_DIR}"/*/ ; do
  processBoat "${boatDir}"
  safeRun "${BIN}"/uploadVmgTable_DockerDev.sh
done

