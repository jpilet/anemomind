#!/bin/bash
#set -e

# This script can be run from crontab with:
#   */5 *  *   *   *   ps aux | grep -v grep | grep processNewLogs || /home/anemomind/bin/processNewLogs.sh

export BIN="/home/anemomind/bin"
export LOG_DIR="/home/anemomind/userlogs/anemologs"
export PROCESSED_DIR="/home/anemomind/processed"
export SRC_ROOT=$(dirname "$0")/../../../
export BUILD_ROOT=${SRC_ROOT}/build

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
  timeout 30m $*
}

processBoat() {

  local boatdir=$1
  local boat=$(basename "${boatdir}")
  local boatid=$(echo "${boat}" | sed 's/boat//')
  local boatprocessdir="${PROCESSED_DIR}/${boat}"
  mkdir -p "${boatprocessdir}"
  local lastprocess="${boatprocessdir}/lastprocess.md5"

  if [ -e "${lastprocess}" ] && ls -lR "${boatdir}" | md5sum | diff -q "${lastprocess}" - ; then
    # checksum OK, nothing to do.
    loginfo "Skipping boat: ${boat}"
    true
  else
    # checksum not present or not up to date: need recompute.

    loginfo "Recomputing for boat: ${boat}"

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

    
    if safeRun "${BUILD_ROOT}"/src/server/nautical/nautical_processBoatLogs \
        ${NOINFO} \
        --dir "${boatprocessdir}" \
        --dst "${processed}" \
        --boatid "${boatid}" \
        -t --clean \
	--db anemomind \
        -u anemomindprod -p asjdhse5sdas \
	--scale 20 ; then

      if [ -f "${boatdat}" ] ; then
        # If a boat.dat file has been generated, mail it to the anemobox.
        cat "${boatdat}" | ssh anemomind@anemolab.com NODE_ENV=production \
          node /home/xa4/anemomind/www2/utilities/SendBoatData.js \
          "${boatid}" /dev/stdin /home/anemobox/boat.dat || true
      fi

      # Recompute worked. Update the checksum.
      ls -lR "${boatdir}" | md5sum > "${lastprocess}"
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

# Make sure we have a ssh tunnel to anemolab DB
#ps aux | grep autossh | grep -q anemolab || (autossh -T -N -L 27017:localhost:27017 jpilet@anemolab.com &)
killall ssh >& /dev/null || true
ssh -C -T -N -L 27017:localhost:27017 jpilet@anemolab.com &
SSH_TUNNEL_PID=$!

# wait for the tunnel to open.
sleep 1

export SHELL=/bin/bash
parallel -j 3 processBoat ::: "${LOG_DIR}/"*

kill $SSH_TUNNEL_PID

safeRun "${BIN}"/uploadVmgTable.sh
