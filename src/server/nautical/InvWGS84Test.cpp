/*
 * InvWGS84Test.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/InvWGS84.h>
#include <server/nautical/WGS84.h>

using namespace sail;

TEST(InvWGS84Test, TestIt) {
  auto lon = Angle<double>::degrees(21.3);
  auto lat = Angle<double>::degrees(27.0);
  auto alt = Length<double>::meters(13.0);

  auto pos = GeographicPosition<double>(lon, lat, alt);

  Length<double> xyz[3];
  WGS84<double>::toXYZ(pos, xyz);

  auto pos2 = computeGeographicPositionFromXYZ(xyz);

  Length<double> xyz2[3];
  WGS84<double>::toXYZ(pos2, xyz2);

  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(xyz[i].meters(), xyz2[i].meters(), 1.0e-5);
  }
}
