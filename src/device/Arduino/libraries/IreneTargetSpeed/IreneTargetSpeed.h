#ifndef IRENE_TARGET_SPEED_H
#define IRENE_TARGET_SPEED_H

float degToGrad(float deg);

float interpolate(const float* array, int arraySize, float x);

// Return the target VMG speed, upwind
float getSpeedUpWind(float tws);

// Return the targe VMG speed, downwind
float getSpeedDownWind(float tws);

// Compare the current VMG with the target VMG.
float getSpeedRatio(float twa, float tws, float gpsSpeed);

#endif  // IRENE_TARGET_SPEED_H
