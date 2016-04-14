/*
 * TimedValueIntegratorTest.cpp
 *
 *  Created on: Apr 14, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/filters/TimedValueIntegrator.h>

using namespace sail;

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

TEST(TimedValueIntegrator, IntegratorTest) {
  auto values = makeTestVector(offset);
  auto itg = TimedValueIntegrator<Velocity<double> >::make(values.begin(), values.end());

  double h = 1.0e-2;

  // Integration
  {
    auto avg0 = itg.computeAverage(offset + (1.0 - h)*seconds,
                                   offset + (1.0 + h)*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 3.0, 1.0e-5);
    EXPECT_NEAR(avg.maxDuration.seconds(), 0, 0.02);
  }{
    auto avg0 = itg.computeAverage(offset + (5.0 - h)*seconds,
                                   offset + (5.0 + h)*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 8.0, 1.0e-5);
    EXPECT_NEAR(avg.maxDuration.seconds(), 0, 0.02);
  }{
    auto avg0 = itg.computeAverage(offset + (7.0 - h)*seconds,
                                   offset + (7.0 + h)*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 5.0, 1.0e-5);
    EXPECT_NEAR(avg.maxDuration.seconds(), 0, 0.02);
  }{
    auto avg0 = itg.computeAverage(offset + (3.0 - h)*seconds,
                                   offset + (3.0 + h)*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 5.5, 1.0e-5);
    EXPECT_NEAR(avg.maxDuration.seconds(), 2.0, 0.02);
  }{ // Out of bounds, take the closest sample
    auto avg0 = itg.computeAverage(offset + (0.5 - h)*seconds,
                                   offset + (0.5 + h)*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 3.0, 1.0e-5);
    EXPECT_NEAR(avg.maxDuration.seconds(), 0.5, 0.02);
  }

  // Point wise
  { // The same point.
    auto t = offset + 5.0*seconds;
    auto avg0 = itg.computeAverage(t, t);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), 8.0, 1.0e-5);
    EXPECT_NEAR(avg.maxDuration.seconds(), 0, 0.02);
  }

  // Integrate larger spans...
  {  // The whole range
    auto avg0 = itg.computeAverage(offset + 1.0*seconds, offset + 7.0*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), (0.5*3 + 1.0*8 + 0.5*5)/2.0, 1.0e-5);
    EXPECT_NEAR(avg.maxDuration.seconds(), 0.0, 1.0e-5);
  }{ // Two samples
    auto avg0 = itg.computeAverage(offset + 1.0*seconds, offset + 5.0*seconds);
    EXPECT_TRUE(avg0.defined());
    auto avg = avg0.get();
    EXPECT_NEAR(avg.value.knots(), (0.5*3 + 0.5*8)/1.0, 1.0e-5);
    EXPECT_NEAR(avg.maxDuration.seconds(), 0.0, 1.0e-5);
  }
}
