#!/bin/bash
#set -e

# This script can be run from crontab with:
#   */5 *  *   *   *   ps aux | grep -v grep | grep processNewLogs || /home/anemomind/bin/processNewLogs.sh

BIN="/home/anemomind/bin"
LOG_DIR="/home/anemomind/userlogs/anemologs"
PROCESSED_DIR="/home/anemomind/processed"

# Make sure we have a ssh tunnel to anemolab DB
#ps aux | grep autossh | grep -q anemolab || (autossh -T -N -L 27017:localhost:27017 jpilet@anemolab.com &)
killall ssh >& /dev/null || true
ssh -T -N -L 27017:localhost:27017 jpilet@anemolab.com &
SSH_TUNNEL_PID=$!

# wait for the tunnel to open.
sleep 1

for boatdir in "${LOG_DIR}/"*; do
  boat=$(basename "${boatdir}")
  boatid=$(echo "${boat}" | sed 's/boat//')
  boatprocessdir="${PROCESSED_DIR}/${boat}"
  mkdir -p "${boatprocessdir}"
  lastprocess="${boatprocessdir}/lastprocess.md5"

  if [ -e "${lastprocess}" ] && ls -lR "${boatdir}" | md5sum | diff -q "${lastprocess}" - ; then
    # checksum OK, nothing to do.
    #echo "Skipping boat: ${boat}"
    true
  else
    # checksum not present or not up to date: need recompute.

    boatdat="${boatprocessdir}/processed/boat.dat"
    [ -f "${boatdat}" ] && rm -f "${boatdat}"

    # log converted to .TXT are not needed anymore, processBoatLogs will read the
    # .log files directly.
    rm -f "${boatprocessdir}/LOG.TXT" || true

    # processBoatLogs takes only 1 arg. Create a symlink to pass the source
    # directory.
    [ -L "${boatprocessdir}/logs" ] || ln -s "${boatdir}" "${boatprocessdir}/logs"

    if "${BIN}"/processBoatLogs --noinfo "${boatprocessdir}" ; then

      # Upload the tiles to the database
      if "${BIN}"/tiles_generateAndUpload \
	--boatDat ${boatdat} \
	--id ${boatid} \
	--navpath "${boatprocessdir}" \
	--table anemomind.tiles \
        --clean \
        --noinfo \
	--scale 20 ; then

        # If a boat.dat file has been generated, mail it to the anemobox.
	if [ -f "${boatdat}" ] ; then
	  cat "${boatdat}" | ssh anemomind@anemolab.com NODE_ENV=production \
            node /home/xa4/anemomind/www2/utilities/SendBoatData.js \
            "${boatid}" /dev/stdin /home/anemobox/boat.dat || true
        fi

        # Recompute worked. Update the checksum.
        ls -lR "${boatdir}" | md5sum > "${lastprocess}"
      else
        echo "tile_generateAndUpload FAILED for ${boatid}"
      fi
    else
      echo "processBoatLogs FAILED for ${boatprocessdir}. Skipping upload."
    fi
  fi
done

kill $SSH_TUNNEL_PID
