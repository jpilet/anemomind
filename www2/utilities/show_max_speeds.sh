#!/bin/bash
set -e
echo "Using NODE_ENV='${NODE_ENV}'"
uri=$(node catconfig.js %s x.mongo.uri)
echo "Using URI='${uri}'"
cat show_max_speeds_mongo.txt | mongo $uri
