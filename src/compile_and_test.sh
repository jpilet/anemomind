#!/bin/bash
set -e

cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_SAILROOT=OFF

# build dependencies
make help | grep ext | sed 's/\.\.\. //' | grep -v gflags | xargs $RUN make -j$(nproc)

make -j$(nproc)
make -j$(nproc) test
