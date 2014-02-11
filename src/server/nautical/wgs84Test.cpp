/*
 *  Created on: 11 févr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <cmath>
#include "wgs84.h"
#include <server/common/math.h>
#include "Ecef.h"

using namespace sail;

// Check that the output XYZ position of this function is reasonable
TEST(wgs84Test, CheckItIsReasonableTest) {
  double lonDeg = 0.3*M_PI;
  double latDeg = 0.4*M_PI;
  double altitudeMetres = 0.0;

  double circumferenceEarthMetres = 40*1.0e6; // 40 000 km
  double radius = circumferenceEarthMetres/(2.0*M_PI);

  double xyz[3];
  wgs84ToXYZGeneric<double>(lonDeg, latDeg, altitudeMetres, xyz, nullptr, nullptr);
  double distFromCog = norm(3, xyz);
  double tol = 0.1;
  EXPECT_LE((1.0 - tol)*radius, distFromCog);
  EXPECT_LE(distFromCog, (1.0 + tol)*radius);
}

// Test if this function does the same thing as the ECEF library. It should.
TEST(wgs84Test, CompareToECEFTest) {
  double lonDeg = 0.3*M_PI;
  double latDeg = 0.4*M_PI;
  double altitudeMetres = 30.0;

  double xyz[3];
  wgs84ToXYZGeneric<double>(lonDeg, latDeg, altitudeMetres, xyz, nullptr, nullptr);

  double ex, ey, ez;
  lla2ecef(deg2rad(lonDeg), deg2rad(latDeg), altitudeMetres, ex, ey, ez);
  EXPECT_NEAR(ex, xyz[0], 1.0e-5);
  EXPECT_NEAR(ey, xyz[1], 1.0e-5);
  EXPECT_NEAR(ez, xyz[2], 1.0e-5);
}

