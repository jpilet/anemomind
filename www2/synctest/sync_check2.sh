#!/bin/bash
set -e
if [[ "${2}" -eq "" ]];
then
    echo "ERROR: No md5 sum to compare against."
    exit 1
else
    WWW2_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
    boatId=$(${WWW2_DIR}/synctest/get_devbox_boatid.sh)
    result=$(md5 -q "${WWW2_DIR}/uploads/anemologs/boat${boatId}/${1}" || true)
    echo "Read this md5 code: '${result}' and truth is '${truth}'"
    truth="${2}"
    if [ "$result" = "$truth" ]; then
        echo "PASSED :-)"
        exit 0
    else
        echo "Failed, got '$result' but expected '$truth'"
        exit 1
    fi
fi
