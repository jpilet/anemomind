#!/bin/bash
set -e

BIN="/home/anemomind/bin"
PROCESSED_DIR="/home/anemomind/processed"
SCRIPT="/tmp/update-vmgtable-$$"

echo > $SCRIPT

for boatdir in "$PROCESSED_DIR/"*; do
  file="$boatdir/processed/boat.dat"
  id="$(basename "${boatdir}"| sed 's/boat//')"

  [ -r $file ] && \
    "${BIN}/catTargetSpeed" --id "${id}" "$file" >> $SCRIPT
done
  
# Authentication for the mongo login is stored on:
# anemolab.com:/home/jpilet/.ssh/authorized_keys
cat $SCRIPT | ssh jpilet@anemolab.com mongo > /dev/null
rm $SCRIPT
