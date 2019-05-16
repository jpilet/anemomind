cd /anemomind/build && \
cmake .. -DCMAKE_BUILD_TYPE=RelWidthDebInfo && \
make -j1 nautical_processBoatLogs logimport_summary anemobox_logcat logimport_try_load nautical_catTargetSpeed
