/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GeographicPosition.h"
#include <gtest/gtest.h>

using namespace sail;

TEST(GeoPosTest, ConstructAndAccessTest) {
  Angle<double> lon = Angle<double>::degrees(30);
  Angle<double> lat = Angle<double>::degrees(49);
  Length<double> alt = Length<double>::meters(300);
  GeographicPosition<double> pos(lon, lat, alt);
  EXPECT_NEAR(pos.lon().radians(), lon.radians(), 1.0e-6);
  EXPECT_NEAR(pos.lat().radians(), lat.radians(), 1.0e-6);
  EXPECT_NEAR(pos.alt().meters(), alt.meters(), 1.0e-6);

  GeographicPosition<float> posf = pos.mapObjectValues([](double x) {
    return static_cast<float>(x);
  });
  EXPECT_NEAR(posf.lon().radians(), lon.radians(), 1.0e-6);
  EXPECT_NEAR(posf.lat().radians(), lat.radians(), 1.0e-6);
  EXPECT_NEAR(posf.alt().meters(), alt.meters(), 1.0e-6);
}


