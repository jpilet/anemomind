#!/bin/bash
#
# Call this script with no argument for the default 'development' mode,
# or an  argument specifying the mode: 'development' or 'production'.
#
set -e # Stop on first error
mode=${1}
if [[ "$mode" -eq "" ]];
then
  mode="development"
fi
echo "Mode is $mode"
WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
endpointDir=$(node ${WWW2_DIR}/utilities/catconfig.js ${mode} "%s" "x.endpointDir")
uploadDir=$(node ${WWW2_DIR}/utilities/catconfig.js ${mode} "%s" "x.uploadDir")
db=$(node ${WWW2_DIR}/utilities/catconfig.js ${mode} "%s" "x.mongo.uri")
( cd ${WWW2_DIR} ; rm -rf ${endpointDir} ; mkdir ${endpointDir} )
( cd ${WWW2_DIR} ; rm -rf ${uploadDir} ; mkdir ${uploadDir} )
cat ${WWW2_DIR}/synctest/resetdb.txt | mongo ${db}
echo "endpointDir = '$endpointDir'"
echo "uploadDir   = '$uploadDir'"
echo "Successfully reset"
