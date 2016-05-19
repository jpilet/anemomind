/*
 * TimedValueIntegratorTest.cpp
 *
 *  Created on: Apr 14, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/filters/TimedValueIntegrator.h>
#include <server/common/LineKM.h>

using namespace sail;


/*
 * EXAMPLE SETUP
 *
 * Samples
 * ((1 seconds, 3 knots), (5 seconds, 8 knots), (7 seconds, 5 knots))
 *
 * We will define a piecewise constant nearest-neighbour function which
 * looks like this (the original samples are marked as x).
 *
 *        Velocity (knots)
 *          ^
 *          |
 *          8                -------------x-----
 *          7
 *          6
 *          5                                   ------x-----
 *          4
 *   -------3-----x-----------
 *          2
 *          1
 *---       0                                               -------------
 *   -1     0     1     2     3     4     5     6     7     8     9    10 ---> Time (seconds)
 *
 *
 * The TimedValueIntegrator class lets us average this function over arbitrary intervals.
 *
 */

namespace {
  auto seconds = Duration<double>::seconds(1.0);
  auto knots = Velocity<double>::knots(1.0);
  auto offset = TimeStamp::UTC(2016, 4, 14, 11, 30, 0);

  TimedValue<Velocity<double> > tv(Duration<double> localTime, Velocity<double> vel) {
    return TimedValue<Velocity<double> >(offset + localTime, vel);
  };

  Array<TimedValue<Velocity<double> > > makeTestVector(TimeStamp offset) {
    return Array<TimedValue<Velocity<double> > >{
      tv(1.0*seconds, 3.0*knots),
      tv(5.0*seconds, 8.0*knots),
      tv(7.0*seconds, 5.0*knots)
    };
  }
}

TEST(TimedValueIntegrator, EmptyData) {
  auto itg = TimedValueIntegrator<Velocity<double> >::make(
      (TimedValue<Velocity<double> >*)nullptr, (TimedValue<Velocity<double> >*)nullptr);
  auto t = TimeStamp::UTC(2016, 4, 14, 17, 16, 0);
  EXPECT_FALSE(itg.computeAverage(t, t + 0.5*seconds).defined());
  EXPECT_FALSE(itg.interpolate(t).defined());
}

TEST(TimedValueIntegrator, OneElement) {
  Array<TimedValue<Velocity<double> > > values{
    tv(0.3*seconds, 9.7*knots)
  };
  auto itg = TimedValueIntegrator<Velocity<double> >::make(
      values.begin(), values.end());
  auto avg0 = itg.computeAverage(offset + 0.29999*seconds,
                                 offset + 0.30001*seconds);
  EXPECT_TRUE(avg0.defined());
  EXPECT_NEAR(avg0.get().value.knots(), 9.7, 1.0e-3);
}

TEST(TimedValueIntegrator, Bounds) {
  auto times = toLocalTimes(makeTestVector(offset),
      Duration<double>::seconds(1.0));
  EXPECT_EQ(times.size(), 3);
  EXPECT_NEAR(times[0], 0.0, 1.0e-5);
  EXPECT_NEAR(times[1], 4.0, 1.0e-5);
  EXPECT_NEAR(times[2], 6.0, 1.0e-5);

  Arrayd bds = computeBounds(times);
  auto expectedBds = Arrayd{-2, 2, 5, 7};
  auto an = bds.size();
  auto bn = expectedBds.size();
  EXPECT_EQ(an, bn);
  for (int i = 0; i < bds.size(); i++) {
    EXPECT_NEAR((bds[i]), (expectedBds[i]), 1.0e-5);
  }
}

TEST(TimedValueIntegrator, BinLookup) {
  {
    Arrayd a{1.0, 3.0};
    EXPECT_EQ(computeBin(a, -234), -1);
    EXPECT_EQ(computeBin(a, -0.5), -1);
    EXPECT_EQ(computeBin(a, 1.0), 0);
    EXPECT_EQ(computeBin(a, 1.5), 0);
    EXPECT_EQ(computeBin(a, 2.5), 0);
    EXPECT_EQ(computeBin(a, 3.0), 1);
    EXPECT_EQ(computeBin(a, 69.0), 1);
  }{
    Arrayd a{7.34, 9.34, 13.4};
    EXPECT_EQ(computeBin(a, 7.1), -1);
    EXPECT_EQ(computeBin(a, 7.34), 0);
    EXPECT_EQ(computeBin(a, 8.0), 0);
    EXPECT_EQ(computeBin(a, 9.34), 1);
    EXPECT_EQ(computeBin(a, 10.0), 1);
    EXPECT_EQ(computeBin(a, 13.4), 2);
    EXPECT_EQ(computeBin(a, 15.0), 2);
  }
}

TEST(TimedValueIntegrator, Cumulative) {
  Arrayd bds{-2, 2, 5, 7};
  auto data = makeTestVector(offset);
  auto cumulative = buildCumulative(bds, data);
  auto expected = Arrayd{3, 15, 39, 49};
  auto m = cumulative.size();
  auto n = expected.size();
  EXPECT_EQ(m, n);
  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(cumulative[i].knots(), expected[i], 1.0e-5);
  }
}

TEST(TimedValueIntegrator, IntegratorTest) {
  auto values = makeTestVector(offset);
  auto itg = TimedValueIntegrator<Velocity<double> >::make(values.begin(), values.end());

  double h = 1.0e-2;

  // Integration
  {
    auto from = offset + (1.0 - h)*seconds;
    auto to = offset + (1.0 + h)*seconds;
    std::cout << "From " << from << " to " << to << std::endl;
    auto avg0 = itg.computeAverage(from, to);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 3.0, 1.0e-5);
    EXPECT_NEAR(avg.duration.seconds(), 0, 0.02);
  }{
    auto avg0 = itg.computeAverage(offset + (5.0 - h)*seconds,
                                   offset + (5.0 + h)*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 8.0, 1.0e-5);
    EXPECT_NEAR(avg.duration.seconds(), 0, 0.02);
  }{
    auto avg0 = itg.computeAverage(offset + (7.0 - h)*seconds,
                                   offset + (7.0 + h)*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 5.0, 1.0e-5);
    EXPECT_NEAR(avg.duration.seconds(), 0, 0.02);
  }{
    auto avg0 = itg.computeAverage(offset + (3.0 - h)*seconds,
                                   offset + (3.0 + h)*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 5.5, 1.0e-5);
    EXPECT_NEAR(avg.duration.seconds(), 2.0, 0.02);
  }{ // Out of bounds, take the closest sample
    auto avg0 = itg.computeAverage(offset + (0.5 - h)*seconds,
                                   offset + (0.5 + h)*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 3.0, 1.0e-5);
    EXPECT_NEAR(avg.duration.seconds(), 0.5, 0.02);
  }

  // Point wise
  { // The same point.
    auto t = offset + 5.0*seconds;
    auto avg0 = itg.computeAverage(t, t);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 8.0, 1.0e-5);
    EXPECT_NEAR(avg.duration.seconds(), 0, 0.02);
  }

  // Integrate larger spans... Look at the figure at the top of this file
  // to understand better.
  {
    auto avg0 = itg.computeAverage(offset - 1.0*seconds, offset + 3.0*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 3.0, 1.0e5);
  }{
    auto avg0 = itg.computeAverage(offset + 3.0*seconds, offset + 6.0*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 8.0, 1.0e5);
  }{
    auto avg0 = itg.computeAverage(offset + 6.0*seconds, offset + 8.0*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 5.0, 1.0e5);
  }{
    auto avg0 = itg.computeAverage(offset + 2.0*seconds, offset + 4.0*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 0.5*(3 + 8), 1.0e5);
  }{
    auto avg0 = itg.computeAverage(offset + 5.0*seconds, offset + 7.0*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 0.5*(8 + 5), 1.0e5);
  }

  {  // The whole range
    auto avg0 = itg.computeAverage(offset - 1.0*seconds, offset + 8.0*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), (4.0*3.0 + 3.0*8.0 + 2.0*5.0)/9.0, 1.0e-5);
    EXPECT_NEAR(avg.duration.seconds(), 2.0, 1.0e-5);
  }
}
