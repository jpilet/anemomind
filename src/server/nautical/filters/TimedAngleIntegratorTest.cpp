/*
 * TimedValueIntegrator<Angle<double> >Test.cpp
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/filters/TimedAngleIntegrator.h>
#include <server/common/ArrayIO.h>

using namespace sail;

TEST(TimedValueIntegratorTest, Avg) {
  auto offset = TimeStamp::UTC(2016, 4, 21, 10, 48, 0);
  auto seconds = Duration<double>::seconds(1.0);
  auto degrees = Angle<double>::degrees(1.0);

  auto tv = [&](double sec, double deg) {
    return TimedValue<Angle<double> >(offset + sec*seconds, deg*degrees);
  };

  Array<TimedValue<Angle<double> > > data{
    tv(1.0, 359), tv(2.0, 1.0), tv(3.0, 359), tv(4.0, 1.0)
  };

  auto itg = TimedValueIntegrator<Angle<double> >::makeFromArray(data);

  EXPECT_NEAR(itg.interpolate(offset + 1.0*seconds).get()
      .value.normalizedAt0().degrees(), -1.0, 1.0e-5);

  EXPECT_NEAR(itg.interpolate(offset + 2.0*seconds).get()
      .value.normalizedAt0().degrees(), 1.0, 1.0e-5);

  EXPECT_NEAR(itg.computeAverage(offset + 1.0*seconds, offset + 4.0*seconds).get()
      .value.normalizedAt0().degrees(), 0.0, 1.0e-5);

  auto marg = 0.1;
  EXPECT_NEAR(itg.computeAverage(offset + (1.0 - marg)*seconds, offset + (4.0 + marg)*seconds).get()
     .value.normalizedAt0().degrees(), 0.0, 1.0e-5);
}
