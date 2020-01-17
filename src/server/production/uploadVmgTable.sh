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
  
cat $SCRIPT | mongo \
	"mongodb://anemomindprod:$MONGO_PASSWORD@anemolab1,anemolab2,compute3,arbiter/anemomind?replicaSet=rs0"
rm $SCRIPT
