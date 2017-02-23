#!/bin/bash
# Make sure that the NODE_ENV environment variable has a meaningful value
# before calling this script, e.g.
#   
#   NODE_ENV=development ./reset_web_server.sh
#
set -e # Stop on first error
if [[ "$NODE_ENV" -eq "" ]];
then
  echo "WARNING: NODE_ENV is empty."
fi
echo "NODE_ENV is '$NODE_ENV'"
WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
endpointDir=$(node ${WWW2_DIR}/utilities/catconfig.js "%s" "x.endpointDir")
uploadDir=$(node ${WWW2_DIR}/utilities/catconfig.js "%s" "x.uploadDir")
db=$(node ${WWW2_DIR}/utilities/catconfig.js "%s" "x.mongo.uri")
( cd ${WWW2_DIR} ; rm -rf ${endpointDir} ; mkdir ${endpointDir} )
( cd ${WWW2_DIR} ; rm -rf ${uploadDir} ; mkdir ${uploadDir} )
cat ${WWW2_DIR}/synctest/resetdb.txt | mongo ${db}
echo "endpointDir = '$endpointDir'"
echo "uploadDir   = '$uploadDir'"
echo "Successfully reset"
