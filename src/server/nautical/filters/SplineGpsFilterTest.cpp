/*
 * SplineGpsFilterTest.cpp
 *
 *  Created on: 31 Oct 2016
 *      Author: jonas
 */


#include <server/nautical/filters/SplineGpsFilter.h>
#include <gtest/gtest.h>
#include <server/common/ArrayBuilder.h>


using namespace sail;

auto offset = TimeStamp::UTC(2016, 11, 1, 8, 17, 0.0);

TEST(SplineGpsFilterTest, FilterIt) {
  ArrayBuilder<TimedValue<GeographicPosition<double>>> positions0;
  for (int i = 0; i < 30; i++) {
    auto t = offset + double(i)*1.0_s;
    auto p = GeographicPosition<double>(34.0_deg + (0.0001*i)*1.0_deg,
        44.0_deg, 0.0_m);
    TimedValue<GeographicPosition<double>> tv(t, p);
    positions0.add(tv);
  }

  auto positions = positions0.get();
  TimeMapper mapper{offset, 2.0_s, 15};

  auto curves = SplineGpsFilter::filter(positions,
      Array<TimedValue<HorizontalMotion<double>>>(),
      Array<TimeMapper>{mapper}, SplineGpsFilter::Settings());

  EXPECT_EQ(curves.size(), 1);
  auto pos = curves[0].evaluateGeographicPosition(offset);
  EXPECT_NEAR(pos.lon().degrees(), 34.0, 0.00001);
}



