#!/bin/sh
# usage: crunch folderName filename.ext

src="./uploads/$1/$2";
dst="./data/$1/$2"
echo "copying $src to $dst...";
#sleep 5
cp $src $dst;
echo "done!"
echo "running node script..."
curl -i -H "Accept: application/json" -X POST -d "id=$1&filename=$2&polar=polar.txt" http://localhost:9000/api/upload/store
echo "done!"
