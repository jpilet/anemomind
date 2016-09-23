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
    offsetTime, Duration<double>::seconds(2.0), 4};
}

/*TEST(BoatStateReconstructor, BasicTest) {
  Array<BoatState<double> > initialStates(1);
  BoatStateReconstructor<double, ServerBoatStateSettings>
    reconstructor(offsetTime, 1.0_s, initialStates);

}*/
