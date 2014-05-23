/*
 *  Created on: May 21, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/GeographicReference.h>
#include <gtest/gtest.h>
#include <server/nautical/WGS84.h>

namespace sail {

TEST(GeographicReferenceTest, UnmapMap) {
  Length<double> xyz[3] = {Length<double>::meters(1.3),
                           Length<double>::meters(30.9),
                           Length<double>::meters(0.3)};

  GeographicReference ref(GeographicPosition<double>(Angle<double>::radians(0.4),
    Angle<double>::radians(0.9), Length<double>::meters(0.3)));

  GeographicPosition<double> pos = ref.unmap(xyz);

  Length<double> xyz2[3];
  ref.map(pos, xyz2);
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(xyz[i].meters(), xyz2[i].meters(), 1.0e-4);
  }
}

TEST(GeographicReferenceTest, ApproxDistancePreservation) {

  GeographicReference ref(GeographicPosition<double>(Angle<double>::radians(0.4),
    Angle<double>::radians(0.9), Length<double>::meters(0.3)));


  for (int i = 0; i < 3; i++) {
    Length<double> xyz[3] = {Length<double>::meters(1.3),
                             Length<double>::meters(30.9),
                             Length<double>::meters(0.3)};
    GeographicPosition<double> pos0 = ref.unmap(xyz);
    xyz[i] = xyz[i] + Length<double>::meters(30.0);
    GeographicPosition<double> pos1 = ref.unmap(xyz);

    Length<double> ecef0[3], ecef1[3];
    WGS84<double>::toXYZ(pos0, ecef0);
    WGS84<double>::toXYZ(pos1, ecef1);
    double dist2 = 0.0;
    for (int j = 0; j < 3; j++) {
      double dif = (ecef0[j] - ecef1[j]).meters();
      dist2 += dif*dif;
    }
    EXPECT_NEAR(sqrt(dist2), 30.0, 1.0e-3);
  }
}


} /* namespace sail */
