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
  dispatcher.publishValue(AWA, "test", Angle<double>::degrees(330));
  dispatcher.publishValue(AWS, "test", Velocity<double>::knots(7.8));
  dispatcher.publishValue(GPS_SPEED, "test", Velocity<double>::knots(3.6));
  dispatcher.publishValue(GPS_BEARING, "test", Angle<double>::degrees(188));

  EXPECT_FALSE(dispatcher.get<VMG>()->dispatcher()->hasValue());
  EXPECT_FALSE(dispatcher.get<TARGET_VMG>()->dispatcher()->hasValue());
  EXPECT_FALSE(dispatcher.get<TWA>()->dispatcher()->hasValue());
  EXPECT_FALSE(dispatcher.get<TWS>()->dispatcher()->hasValue());

  estimator.compute();

  EXPECT_TRUE(dispatcher.get<VMG>()->dispatcher()->hasValue());
  EXPECT_TRUE(dispatcher.get<TARGET_VMG>()->dispatcher()->hasValue());
  EXPECT_TRUE(dispatcher.get<TWA>()->dispatcher()->hasValue());
  EXPECT_TRUE(dispatcher.get<TWS>()->dispatcher()->hasValue());

  EXPECT_NEAR(4.9, dispatcher.val<TWS>().knots(), .5);
  EXPECT_NEAR(315, dispatcher.val<TWA>().degrees(), 20);

  // High tolerance because those value may vary. We do not have ground truth.
  EXPECT_NEAR(1.8, dispatcher.val<VMG>().knots(), 2);
  EXPECT_NEAR(3.9, dispatcher.val<TARGET_VMG>().knots(), 2);
}



