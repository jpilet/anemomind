#include "IreneTargetSpeed.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415927
#endif

namespace {

// Generated by save_pres.m
const float upWindSpeedTable[] = {
  0.688243,  // 0.000000 - 1.000000
  1.056177,  // 1.000000 - 2.000000
  1.197781,  // 2.000000 - 3.000000
  1.824046,  // 3.000000 - 4.000000
  1.950740,  // 4.000000 - 5.000000
  2.673579,  // 5.000000 - 6.000000
  3.251034,  // 6.000000 - 7.000000
  3.715724,  // 7.000000 - 8.000000
  4.135443,  // 8.000000 - 9.000000
  4.374957,  // 9.000000 - 10.000000
  4.601959,  // 10.000000 - 11.000000
  4.771937,  // 11.000000 - 12.000000
  4.885184,  // 12.000000 - 13.000000
  5.020241,  // 13.000000 - 14.000000
  5.108197,  // 14.000000 - 15.000000
  5.199103,  // 15.000000 - 16.000000
  5.199103,  // 16.000000 - 17.000000
  5.366182,  // 17.000000 - 18.000000
  5.361093,  // 18.000000 - 19.000000
  5.350234,  // 19.000000 - 20.000000
  5.346039,  // 20.000000 - 21.000000
  5.255547,  // 21.000000 - 22.000000
  5.284510,  // 22.000000 - 23.000000
  5.436043,  // 23.000000 - 24.000000
  5.742351,  // 24.000000 - 25.000000
};

// Generated by save_portant.m
const float downWindSpeedTable[] = {
  0.484810,  // 0.000000 - 1.000000
  1.159358,  // 1.000000 - 2.000000
  1.895836,  // 2.000000 - 3.000000
  1.833750,  // 3.000000 - 4.000000
  2.908575,  // 4.000000 - 5.000000
  3.364416,  // 5.000000 - 6.000000
  3.561400,  // 6.000000 - 7.000000
  4.067981,  // 7.000000 - 8.000000
  4.681337,  // 8.000000 - 9.000000
  4.893591,  // 9.000000 - 10.000000
  5.196052,  // 10.000000 - 11.000000
  5.340053,  // 11.000000 - 12.000000
  5.459004,  // 12.000000 - 13.000000
  5.629165,  // 13.000000 - 14.000000
  5.987986,  // 14.000000 - 15.000000
  6.344155,  // 15.000000 - 16.000000
  6.484031,  // 16.000000 - 17.000000
  6.746212,  // 17.000000 - 18.000000
  6.973363,  // 18.000000 - 19.000000
  7.087929,  // 19.000000 - 20.000000
  7.207038,  // 20.000000 - 21.000000
  7.362800,  // 21.000000 - 22.000000
  7.468120,  // 22.000000 - 23.000000
  7.730803,  // 23.000000 - 24.000000
  7.752514,  // 24.000000 - 25.000000
};

}  // namespace

float degToGrad(float deg) {
  return float(deg * (M_PI / 180.0));
}

float interpolate(const float* array, int arraySize, float x) {
  if (x <= 0) {
    return array[0];
  }
  int x_lower = (int) x;
  float remaining = x - (float) x_lower;

  if (x_lower >= arraySize - 1) {
    return array[arraySize - 1];
  }

  return array[x_lower] * (1.0f - remaining) + remaining * array[x_lower + 1];
}

float getSpeedUpWind(float tws) {
  return interpolate(upWindSpeedTable, sizeof(upWindSpeedTable) / sizeof(upWindSpeedTable[0]), tws);
}

float getSpeedDownWind(float tws) {
  return interpolate(downWindSpeedTable, sizeof(downWindSpeedTable) / sizeof(downWindSpeedTable[0]), tws);
}

float getSpeedRatio(float twa, float tws, float gpsSpeed) {
  float vmgGps = cos(degToGrad(twa)) * gpsSpeed;
  float refSpeed;
  if (vmgGps > 0) {
    refSpeed = getSpeedUpWind(tws);
  } else {
    vmgGps = -vmgGps;
    refSpeed = getSpeedDownWind(tws);
  }

  if (refSpeed < .1f)
    return 0.0f;

  return vmgGps / refSpeed;
}
