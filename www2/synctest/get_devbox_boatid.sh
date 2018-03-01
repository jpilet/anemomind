#!/bin/bash
boxid=$(cat boxid.txt)
mongo --quiet anemomind-dev --eval "db.boats.findOne({anemobox: '${boxid}'})._id.valueOf()"
