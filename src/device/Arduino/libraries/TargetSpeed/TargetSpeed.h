// (c) 2014 Julien Pilet <julien.pilet@gmail.com>

#ifndef DEVICE_TARGET_SPEED_H
#define DEVICE_TARGET_SPEED_H

#include "../FixedPoint/FixedPoint.h"
#include "../PhysicalQuantity/PhysicalQuantity.h"

namespace sail {

struct TargetSpeedTable {
  enum {
    VERSION=1,
    STRUCT_IDENTIFIER=0x7392,
    NUM_ENTRIES=32,
  };
  FP8_8 _upwind[NUM_ENTRIES];
  FP8_8 _downwind[NUM_ENTRIES];

  FP8_8 binLowerBound(int bin) const { return FP8_8(bin); }
  FP8_8 binHightBound(int bin) const { return FP8_8(bin + 1); }
  FP8_8 binCenter(int bin) const { return FP8_8(bin) + FP8_8(.5); }
};

float getVmgSpeedRatio(const TargetSpeedTable& table,
                       short twa, FP8_8 tws, FP8_8 gpsSpeed);

void invalidateSpeedTable(TargetSpeedTable *table);

Velocity<> getVmgTarget(const TargetSpeedTable& table,
                        Angle<> twa, Velocity<> tws);

#ifdef ON_SERVER
//! Reads the 'boat.dat' file from disc, and fill the given TargetSpeedTable
//  structure. Returns true on success, false on failure.
bool loadTargetSpeedTable(const char *filename, TargetSpeedTable *table);

//! Output the given target speed as JSON on stdout.
//  invalid values are ignored.
void printTargetSpeedAsJson(const TargetSpeedTable& table);

void printTargetSpeedAsCsv(const TargetSpeedTable& table);

#endif

}  // namespace sail

#endif // DEVICE_TARGET_SPEED_H
