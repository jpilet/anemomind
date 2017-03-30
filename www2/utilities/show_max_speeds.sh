#!/bin/bash
set -e
echo "Using NODE_ENV='${NODE_ENV}'"
cat show_max_speeds_mongo.txt | mongo "${NODE_ENV}"
