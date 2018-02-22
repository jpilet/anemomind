#!/bin/bash
boxid=$(cat boxid.txt)
echo "Looking up boat for box '${boxid}'"
mongo --quiet anemomind-dev --eval 'db.boats.findOne({anemobox: "${boxid}"})["_id"].valueOf()'
