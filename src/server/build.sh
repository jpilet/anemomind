#!/bin/bash
set -e
SCRIPT_DIR=$(dirname "$( which "$0" )")
SOURCE_DIR="$( cd ${SCRIPT_DIR}/../../ && pwd )"
BUILD_DIR=${1:pwd}
cd "${BUILD_DIR}"
cmake "${SOURCE_DIR}" -DCMAKE_BUILD_TYPE=RelWidthDebInfo -DWITH_SAILROOT=OFF && \
make -j1 nautical_processBoatLogs logimport_summary anemobox_logcat logimport_try_load nautical_catTargetSpeed
