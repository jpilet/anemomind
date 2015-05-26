#include <device/anemobox/DispatcherTrueWindEstimator.h>
#include <device/anemobox/FakeClockDispatcher.h>
#include <gtest/gtest.h>
#include <server/common/Env.h>

using namespace sail;

TEST(DispatcherTrueWindEstimatorTest, singleValTest) {
  FakeClockDispatcher dispatcher;
  DispatcherTrueWindEstimator estimator(&dispatcher);

  EXPECT_TRUE(estimator.loadCalibration(
          std::string(Env::SOURCE_DIR)
          + std::string("/src/device/Arduino/NMEAStats/test/boat.dat")));

  // Values taken from NmeaStatsTest.cpp
  dispatcher.awa()->publishValue("test", Angle<double>::degrees(330));
  dispatcher.aws()->publishValue("test", Velocity<double>::knots(7.8));
  dispatcher.gpsSpeed()->publishValue("test", Velocity<double>::knots(3.6));
  dispatcher.gpsBearing()->publishValue("test", Angle<double>::degrees(188));

  EXPECT_FALSE(dispatcher.vmg()->dispatcher()->hasValue());
  EXPECT_FALSE(dispatcher.targetVmg()->dispatcher()->hasValue());
  EXPECT_FALSE(dispatcher.twa()->dispatcher()->hasValue());
  EXPECT_FALSE(dispatcher.tws()->dispatcher()->hasValue());

  estimator.compute();

  EXPECT_TRUE(dispatcher.vmg()->dispatcher()->hasValue());
  EXPECT_TRUE(dispatcher.targetVmg()->dispatcher()->hasValue());
  EXPECT_TRUE(dispatcher.twa()->dispatcher()->hasValue());
  EXPECT_TRUE(dispatcher.tws()->dispatcher()->hasValue());

  EXPECT_NEAR(4.9, dispatcher.tws()->dispatcher()->lastValue().knots(), .5);
  EXPECT_NEAR(315, dispatcher.twa()->dispatcher()->lastValue().degrees(), 20);

  // High tolerance because those value may vary. We do not have ground truth.
  EXPECT_NEAR(1.8, dispatcher.vmg()->dispatcher()->lastValue().knots(), 2);
  EXPECT_NEAR(3.9, dispatcher.targetVmg()->dispatcher()->lastValue().knots(), 2);
}



