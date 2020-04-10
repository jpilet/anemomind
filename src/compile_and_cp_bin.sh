#!/bin/bash
set -e

cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_SAILROOT=OFF

# build dependencies
make help | grep ext | sed 's/\.\.\. //' \
       | grep -v gflags | grep -v gtest \
       | xargs make -j$(nproc)

make -j$(nproc) \
    nautical_processBoatLogs logimport_summary \
    anemobox_logcat logimport_try_load nautical_catTargetSpeed

TARGETS="./src/server/nautical/nautical_catTargetSpeed ./src/server/nautical/nautical_processBoatLogs ./src/server/nautical/logimport/logimport_try_load"

mkdir -p ../bin
mkdir -p ../lib
cp $(ldd $TARGETS | grep -o '/.\+\.so[^ ]*' | sort | uniq) ../lib
cp $TARGETS ../bin
