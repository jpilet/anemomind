#include <device/anemobox/DispatcherFilter.h>
#include <device/anemobox/FakeClockDispatcher.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace sail;

TEST(DispatcherFilterTest, singleValTest) {
  FakeClockDispatcher dispatcher;

  dispatcher.awa()->publishValue("test", Angle<double>::degrees(-3));
  dispatcher.aws()->publishValue("test", Velocity<double>::knots(3));

  DispatcherFilter filter(&dispatcher, DispatcherFilterParams());

  double tolerance = 1e-5;
  EXPECT_NEAR(-3, filter.awa().degrees(), tolerance);
  EXPECT_NEAR(3, filter.aws().knots(), tolerance);
}

TEST(DispatcherFilterTest, changingValTest) {
  FakeClockDispatcher dispatcher;
  DispatcherFilter filter(&dispatcher, DispatcherFilterParams());

  // 2 minutes with value A
  for (int i = 0; i < 120; ++i) {
    dispatcher.magHdg()->publishValue("test", Angle<double>::degrees(-3));
    dispatcher.watSpeed()->publishValue("test", Velocity<double>::knots(3));
    dispatcher.advance(Duration<>::seconds(1));
  }

  double tolerance = 1e-5;
  EXPECT_NEAR(-3, filter.magHdg().degrees(), tolerance);
  EXPECT_NEAR(3, filter.watSpeed().knots(), tolerance);

  // 20 seconds with value B
  for (int i = 0; i < 20; ++i) {
    dispatcher.magHdg()->publishValue("test", Angle<double>::degrees(173));
    dispatcher.watSpeed()->publishValue("test", Velocity<double>::knots(23));
    dispatcher.advance(Duration<>::seconds(1));
  }

  // Value A should be filtered out.
  EXPECT_NEAR(173, filter.magHdg().degrees(), tolerance);
  EXPECT_NEAR(23, filter.watSpeed().knots(), tolerance);
}

TEST(DispatcherFilterTest, noisyValueTest) {
  FakeClockDispatcher dispatcher;
  DispatcherFilter filter(&dispatcher, DispatcherFilterParams());

  // values with oscillations
  for (int i = 0; i < 120; ++i) {
    dispatcher.magHdg()->publishValue(
        "test", Angle<double>::degrees(90 + sin(i * .7213) * 5));
    dispatcher.watSpeed()->publishValue(
        "test", Velocity<double>::knots(7 + sin(i * .343)));
    dispatcher.advance(Duration<>::seconds(.1));
  }

  EXPECT_NEAR(90, filter.magHdg().degrees(), .5);
  EXPECT_NEAR(7, filter.watSpeed().knots(), .1);
}

