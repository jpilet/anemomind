#!/bin/bash
mongo --quiet anemomind-dev --eval 'db.boats.findOne({anemobox: "784b87a0b162"})["_id"].valueOf()'
