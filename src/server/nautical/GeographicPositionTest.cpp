/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GeographicPosition.h"
#include <gtest/gtest.h>

using namespace sail;

TEST(GeoPosTest, ConstructAndAccessTest) {
  Angle<double> lon = Angle<double>::fromDegrees(30);
  Angle<double> lat = Angle<double>::fromDegrees(49);
  Length<double> alt = Length<double>::fromMetres(300);
  GeographicPosition<double> pos(lon, lat, alt);
  EXPECT_NEAR(pos.lon().toRadians(), lon.toRadians(), 1.0e-6);
  EXPECT_NEAR(pos.lat().toRadians(), lat.toRadians(), 1.0e-6);
  EXPECT_NEAR(pos.alt().toMetres(), alt.toMetres(), 1.0e-6);
}


