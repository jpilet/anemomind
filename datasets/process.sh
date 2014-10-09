#!/bin/bash

set -e

BIN_DIR="../build"
[ -e "../build_release" ] && BIN_DIR="../build_release"

NAUTICAL="${BIN_DIR}/src/server/nautical"

for boat in Irene; do #exocet psaros33_Banque_Sturdza; do
  #echo "Processing boat logs for $boat"
  #"${NAUTICAL}/nautical_processBoatLogs" "$boat"
  #echo "Generating polar for $boat"
  #"${NAUTICAL}/polar/nautical_polar_FilteredPolarExample" --navpath "$boat" --save "$boat/processed/filtered.json"
  #"${NAUTICAL}/polar/nautical_polar_FilteredPolarExample" --view-spans "$boat/processed/filtered.json" 400 \
  #  --optimize 7 32 --build-table "${boat}/processed/polar.dat"
  echo Generating tiles.
  "${NAUTICAL}/tiles/tiles_generateAndUpload" --id $boat --maxpoints 32 --navpath $boat --scale 20 \
    --polarDat "${boat}/processed/polar.dat" --boatDat "${boat}/processed/boat.dat"
done



