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

TEST(GeoCalcTest, RefTest) {
  GeographicPosition<double> a(Angle<double>::degrees(30.0),
                                 Angle<double>::degrees(30),
                                 Length<double>::meters(60));
  GeographicPosition<double> b(Angle<double>::degrees(30.01),
                                 Angle<double>::degrees(30),
                                 Length<double>::meters(60));

  Length<double> xyzA[3], xyzB[3];
  double xyzAm[3], xyzBm[3];

  WGS84<double>::toXYZ(a, xyzA);
  WGS84<double>::toXYZ(b, xyzB);
  for (int i = 0; i < 3; i++) {
    xyzAm[i] = xyzA[i].meters();
    xyzBm[i] = xyzB[i].meters();
  }

  double dist = normdif<double, 3>(xyzAm, xyzBm);

  GeographicReference ref(a);
  Length<double> xy[2];
  ref.project<double>(b, xy);
  double dist2 = sqrt(sqr(xy[0].meters()) + sqr(xy[1].meters()));

  EXPECT_NEAR((dist - dist2)/dist, 0.0, 1.0e-3);
}
