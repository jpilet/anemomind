#!/bin/bash
set -e

#BIN="/home/anemomind/bin"
#PROCESSED_DIR="/home/anemomind/processed"
#SCRIPT="/tmp/update-vmgtable-$$"

BIN="/anemomind/bin"
PROCESSED_DIR="/db/processed"
SCRIPT="/tmp/update-vmgtable-$$"

echo > $SCRIPT

for boatdir in "$PROCESSED_DIR/"*; do
  file="$boatdir/processed/boat.dat"
  id="$(basename "${boatdir}"| sed 's/boat//')"

  [ -r $file ] && \
    "${BIN}/nautical_catTargetSpeed" --id "${id}" "$file" >> $SCRIPT
    # "${BIN}/catTargetSpeed" --id "${id}" "$file" >> $SCRIPT
done
  
cat $SCRIPT | mongo \
      ${MONGO_URL}
rm $SCRIPT
