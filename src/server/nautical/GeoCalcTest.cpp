/*
 *  Created on: 24 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include "GeoCalc.h"
#include <server/nautical/WGS84.h>
#include <server/common/string.h>

using namespace sail;

TEST(GeoCalcTest, MapAndInvert) {
  GeographicPosition<double> pos(Angle<double>::degrees(30),
                                 Angle<double>::degrees(30),
                                 Length<double>::meters(60));
  Length<double> XYZ[3];
  WGS84<double>::toXYZ(pos, XYZ);

  GeographicPosition<double> pos2 = toGeographicPosition(XYZ);

  Length<double> XYZ2[3];
  WGS84<double>::toXYZ(pos2, XYZ2);


  Length<double> tol = Length<double>::meters(0.1);
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(XYZ[i].meters(), XYZ2[i].meters(), tol.meters());
  }
}

TEST(GeoCalcTest, Mean2) {
  GeographicPosition<double> pos(Angle<double>::degrees(30),
                                 Angle<double>::degrees(30),
                                 Length<double>::meters(60));

  Array<GeographicPosition<double> > positions(2);
  positions[0] = pos;
  positions[1] = pos;

  GeographicPosition<double> meanPos = mean(positions);
  Length<double> XYZ[3], XYZ2[3];
  WGS84<double>::toXYZ(pos, XYZ);
  WGS84<double>::toXYZ(pos, XYZ2);
  Length<double> tol = Length<double>::meters(0.1);
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(XYZ[i].meters(), XYZ2[i].meters(), tol.meters());
  }
}

TEST(GeoCalcTest, Mean2dif) {
  GeographicPosition<double> refMeanPos(Angle<double>::degrees(30.05),
                                 Angle<double>::degrees(30),
                                 Length<double>::meters(60));

  Array<GeographicPosition<double> > positions(2);
  positions[0] = GeographicPosition<double>(Angle<double>::degrees(30),
      Angle<double>::degrees(30),
      Length<double>::meters(60));
  positions[1] = GeographicPosition<double>(Angle<double>::degrees(30.1),
      Angle<double>::degrees(30),
      Length<double>::meters(60));

  GeographicPosition<double> meanPos = mean(positions);
  Length<double> XYZ[3], XYZ2[3];
  WGS84<double>::toXYZ(meanPos, XYZ);
  WGS84<double>::toXYZ(refMeanPos, XYZ2);
  Length<double> tol = Length<double>::meters(2.0);
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(XYZ[i].meters(), XYZ2[i].meters(), tol.meters());
  }
}
