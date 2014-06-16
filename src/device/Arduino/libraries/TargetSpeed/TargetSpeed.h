// (c) 2014 Julien Pilet <julien.pilet@gmail.com>

#ifndef DEVICE_TARGET_SPEED_H
#define DEVICE_TARGET_SPEED_H

#include "FixedPoint.h"

struct TargetSpeedTable {
  enum {
    VERSION=1,
    STRUCT_IDENTIFIER=0x7392,
    NUM_ENTRIES=32,
  };
  FP8_8 _upwind[NUM_ENTRIES];
  FP8_8 _downwind[NUM_ENTRIES];
};

float getVmgSpeedRatio(const TargetSpeedTable& table,
                       short twa, FP8_8 tws, FP8_8 gpsSpeed);

void invalidateSpeedTable(TargetSpeedTable *table);

#endif // DEVICE_TARGET_SPEED_H
