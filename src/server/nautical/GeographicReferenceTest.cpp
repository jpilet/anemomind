/*
 *  Created on: May 21, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/GeographicReference.h>
#include <gtest/gtest.h>
#include <server/nautical/WGS84.h>

namespace sail {

TEST(GeographicReferenceTest, UnmapMap) {
  GeographicReference::ProjectedPosition xy{Length<double>::meters(1.3), Length<double>::meters(30.9)};

  GeographicReference ref(GeographicPosition<double>(Angle<double>::radians(0.4),
    Angle<double>::radians(0.9), Length<double>::meters(0.3)));

  GeographicPosition<double> pos = ref.unmap(xy);

  GeographicReference::ProjectedPosition xy2 = ref.map(pos);
  for (int i = 0; i < 2; i++) {
    EXPECT_NEAR(xy[i].meters(), xy2[i].meters(), 1.0e-4) << " for i=" << i;
  }
}

TEST(GeographicReferenceTest, ApproxDistancePreservation) {

  GeographicReference ref(GeographicPosition<double>(Angle<double>::radians(0.4),
    Angle<double>::radians(0.9), Length<double>::meters(0.3)));


  for (int i = 0; i < 2; i++) {
    GeographicReference::ProjectedPosition xy{Length<double>::meters(1.3),
                                              Length<double>::meters(30.9)};
    GeographicPosition<double> pos0 = ref.unmap(xy);
    xy[i] = xy[i] + Length<double>::meters(30.0);
    GeographicPosition<double> pos1 = ref.unmap(xy);

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
