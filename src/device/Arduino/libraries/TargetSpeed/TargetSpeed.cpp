// (c) 2014 Julien Pilet <julien.pilet@gmail.com>

#include "TargetSpeed.h"

#include <math.h>

namespace {

float degToGrad(float deg) {
  return float(deg * (M_PI / 180.0));
}

FP8_8 interpolate(const FP8_8* array, FP8_8 x) {
  if (x <= FP8_8(0)) {
    return array[0];
  }
  int x_lower = (int) x;
  FP8_8 remaining = x - FP8_8(x_lower);

  const int lastIndex = TargetSpeedTable::NUM_ENTRIES - 1;
  if (x_lower >= lastIndex) {
    return array[lastIndex];
  }

  return array[x_lower] * (FP8_8(1) - remaining) + remaining * array[x_lower + 1];
}

FP8_8 getSpeedUpWind(const TargetSpeedTable& table, FP8_8 tws) {
  return interpolate(table._upwind, tws);
}

float getSpeedDownWind(const TargetSpeedTable& table, FP8_8 tws) {
  return interpolate(table._downwind, tws);
}

} // namespace

float getVmgSpeedRatio(const TargetSpeedTable& table,
                       float twa, float tws, float gpsSpeed) {
  float vmgGps = cos(degToGrad(twa)) * gpsSpeed;
  float refSpeed;
  if (vmgGps > 0) {
    refSpeed = getSpeedUpWind(table, tws);
  } else {
    vmgGps = -vmgGps;
    refSpeed = getSpeedDownWind(table, tws);
  }

  if (refSpeed < .1f)
    return 0.0f;

  return vmgGps / refSpeed;
}

