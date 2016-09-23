/*
 * BoatStateReconstructorTest.cpp
 *
 *  Created on: 22 Sep 2016
 *      Author: jonas
 */

#include <server/nautical/calib/BoatStateReconstructor.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {
  TimeStamp offsetTime = TimeStamp::UTC(2016, 9, 22, 15, 17, 0);

}

TEST(BoatStateReconstructor, TimeMapper) {
  TimeStampToIndexMapper mapper{
    offsetTime, 2.0_s, 4};
  EXPECT_EQ(mapper.offset, offsetTime);
  EXPECT_EQ(mapper.period, 2.0_s);
  EXPECT_EQ(mapper.sampleCount, 4);
  EXPECT_EQ(mapper.map(offsetTime - 1.0_s), -1);
  EXPECT_EQ(mapper.map(offsetTime - 9.0_s), -1);
  EXPECT_EQ(mapper.map(offsetTime + 1.0_s), 0);
  EXPECT_EQ(mapper.map(offsetTime + 3.0_s), 1);
  EXPECT_EQ(mapper.map(offsetTime + 9.0_s), -1);
}

/*TEST(BoatStateReconstructor, BasicTest) {
  Array<BoatState<double> > initialStates(1);
  BoatStateReconstructor<double, ServerBoatStateSettings>
    reconstructor(offsetTime, 1.0_s, initialStates);

}*/
