/*
 *  Created on: May 21, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/GeographicReference.h>
#include <gtest/gtest.h>

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



} /* namespace sail */
