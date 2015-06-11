#!/bin/bash
set -e

# This script can be run from crontab with:
#   */5 *  *   *   *   ps aux | grep -v grep | grep processNewLogs || /home/anemomind/bin/processNewLogs.sh

BIN="/home/anemomind/bin"
LOG_DIR="/home/anemomind/userlogs/anemologs"
PROCESSED_DIR="/home/anemomind/processed"

# Make sure we have a ssh tunnel to anemolab DB
ps aux | grep autossh | grep -q anemolab || (autossh -T -N -L 27017:localhost:27017 jpilet@anemolab.com &)
 
for boatdir in "${LOG_DIR}/"*; do
  boat=$(basename "${boatdir}")
  boatprocessdir="${PROCESSED_DIR}/${boat}"
  mkdir -p "${boatprocessdir}"
  lastprocess="${boatprocessdir}/lastprocess.md5"

  if [ -e "${lastprocess}" ] && ls -lR "${boatdir}" | md5sum | diff -q "${lastprocess}" - ; then
    # checksum OK, nothing to do.
    echo "Skipping boat: ${boat}"
  else
    # checksum not present or not up to date: need recompute.
    ls -lR "${boatdir}" | md5sum > "${lastprocess}"

    # HACK: processBoatLogs can't deal with "log" files yet. Convert to NMEA first.
    "${BIN}"/logcat "${boatdir}"/*log > "${boatprocessdir}/LOG.TXT"
    if "${BIN}"/processBoatLogs "${boatprocessdir}" ; then

      # Upload the tiles to the database
      "${BIN}"/tiles_generateAndUpload \
        --boatDat "${boatprocessdir}/processed/boat.dat" \
	--id $(echo "${boat}" | sed 's/boat//') \
	--navpath "${boatprocessdir}" \
	--table anemomind.tiles
    else
      echo "processBoatLogs FAILED for ${boatprocessdir}. Skipping upload."
    fi
  fi
done
