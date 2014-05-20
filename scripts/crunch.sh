#!/bin/sh
# usage: crunch folderName filename.ext
set -e
src="${PWD}/uploads/$1/$2";
dst="${PWD}/data/$1/$2"
echo "copying $src to $dst...";
#sleep 5
cp $src $dst;

# Please make sure that the crunching program is on the path
# either by adding its directory to the path or by
# copying the executable to a directory that is on the path.
#
# On the anemolab server, the file is located here:
# /home/jpilet/anemomind/build/src/server/nautical/nautical_processBoatLogs
/home/jpilet/anemomind/build/src/server/nautical/nautical_processBoatLogs ${PWD}/data/$1 $2

echo "done!"
echo "running node script..."
curl -i -H "Accept: application/json" -X POST -d "id=$1&filename=$2&polar=polar.txt" http://localhost/api/upload/store
echo "done!"
