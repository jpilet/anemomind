#!/bin/bash
mongo --quiet anemomind-dev --eval 'db.boats.find({anemobox: "784b87a0b162"}).toArray()[0]["_id"].valueOf()'
