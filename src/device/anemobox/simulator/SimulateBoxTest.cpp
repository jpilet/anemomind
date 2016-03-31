#include <gtest/gtest.h>
#include <device/anemobox/simulator/SimulateBox.h>
#include <device/anemobox/DispatcherUtils.h>

using namespace sail;

namespace {

  template <typename T>
  void pt(ReplayDispatcher2 *dst, DataCode c, Duration<double> localTime, T x) {
    auto start = TimeStamp::UTC(2016, 3, 24, 18, 10, 0);
    const char *name = "test";
    TimeStamp t = start + localTime;

    if (dst->currentTime().defined()) {
      EXPECT_LE(dst->currentTime(), t);
    }

    dst->publishTimedValue<T>(c, name, TimedValue<T>(t, x));
  }
}

// This test checks that the true wind estimator will get called when it should,
// when we are doing a replay.
TEST(SimulateBox, Replay) {

  ReplayDispatcher2 d;

  /*    AWA, AWS, GPS_SPEED, GPS_BEARING,
      WAT_SPEED, MAG_HEADING*/
  auto knots = Velocity<double>::knots(1.0);
  auto degrees = Angle<double>::degrees(1.0);
  auto ms = Duration<double>::milliseconds(1.0);

  typedef Angle<double> A;
  typedef Velocity<double> V;

  // First a dense batch of values. It should trigger on the last one of those.
  pt<A>(&d, AWA, 0.0*ms, 3.0*degrees);
  pt<V>(&d, AWS, 1.0*ms, 12.0*knots);
  pt<V>(&d, GPS_SPEED, 2.0*ms, 4.0*knots);
  pt<A>(&d, GPS_BEARING, 3.0*ms, 34.0*degrees);
  pt<V>(&d, WAT_SPEED, 4.0*ms, 4.0*knots);
  pt<A>(&d, MAG_HEADING, 4.0*ms, 35.0*degrees);
  pt<A>(&d, AWA, 4.0*ms, 3.0*degrees);
  pt<V>(&d, AWS, 5.0*ms, 12.0*knots);
  pt<V>(&d, GPS_SPEED, 5.0*ms, 4.0*knots);
  pt<A>(&d, GPS_BEARING, 5.0*ms, 34.0*degrees);
  pt<V>(&d, WAT_SPEED, 6.0*ms, 4.0*knots);
  pt<A>(&d, MAG_HEADING, 7.0*ms, 35.0*degrees); // <-- 12th value. Trigger compute() here.

  // Gap of more than 20 ms

  pt<A>(&d, AWA, 3000.0*ms, 3.0*degrees); // <-- 13th value. Trigger compute() here.

  // This is a stack of the number of values that the destination dispatcher
  // will contain when it is called. So for the first call, it will contain 12
  // values, and for the second call it will contain 13.
  std::vector<int> expectedCounts{13, 12};


  ReplayDispatcher2 d2;
  replayDispatcherTrueWindEstimator(&d, &d2, [&]() {
      EXPECT_FALSE(expectedCounts.empty());

      auto e = expectedCounts.back();
      expectedCounts.pop_back();
      
      EXPECT_EQ(e, countValues(&d2));
    });

  EXPECT_TRUE(expectedCounts.empty());

  EXPECT_TRUE(true);
}
