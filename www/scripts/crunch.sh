#!/bin/sh
# usage: crunch folderName filename.ext
set -e
srcdir="${PWD}/uploads/$1";
dstdir="${PWD}/data/$1"
mkdir -p "${PWD}/data/$1"
echo "copying $srcdir/* to $dstdir...";
#sleep 5
cp "${srcdir}"/* "${dstdir}";

# Please make sure that the crunching program is on the path
# either by adding its directory to the path or by
# copying the executable to a directory that is on the path.
#
# On the anemolab server, the file is located here:
# ./build/src/server/nautical/nautical_processBoatLogs
# After a git clone, the application must be compiled like so:
# make nautical_processBoatLogs
../build/src/server/nautical/nautical_processBoatLogs "${PWD}/data/${1}"

echo "done!"
echo "running node script..."
curl -i -H "Accept: application/json" -X POST -d "id=$1&filename=$2&polar=polar.txt" http://127.0.0.1:8080/api/upload/store
echo "done!"
