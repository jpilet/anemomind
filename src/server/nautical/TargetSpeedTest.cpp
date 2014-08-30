/*
 *  Created on: 2014-08-30
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/nautical/TargetSpeed.h>
#include <gtest/gtest.h>



namespace sail {
int lookUpForUnitTest(Array<Velocity<double> > bounds, Velocity<double> tws);
}

using namespace sail;
TEST(TargetSpeedTest, LookUpTest) {
  Array<Velocity<double> > bounds(3);
  for (int i = 0; i < bounds.size(); i++) {
    bounds[i] = Velocity<double>::knots(i);
  }

  EXPECT_EQ(lookUpForUnitTest(bounds, Velocity<double>::knots(-0.1)), -1);
  EXPECT_EQ(lookUpForUnitTest(bounds, Velocity<double>::knots(0.1)), 0);
  EXPECT_EQ(lookUpForUnitTest(bounds, Velocity<double>::knots(1.1)), 1);
  EXPECT_EQ(lookUpForUnitTest(bounds, Velocity<double>::knots(2.1)), 2);
  EXPECT_EQ(lookUpForUnitTest(bounds, Velocity<double>::knots(3.1)), -1);


}



