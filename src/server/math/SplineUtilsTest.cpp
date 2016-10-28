/*
 * SplineUtilsTest.cpp
 *
 *  Created on: 28 Oct 2016
 *      Author: jonas
 */

#include <server/math/SplineUtils.h>
#include <gtest/gtest.h>

using namespace sail;


TEST(SplineUtilsTest, TemporalTest) {
  auto offset = TimeStamp::UTC(2016, 5, 4, 3, 3, 4);

  Array<TimeStamp> src{
    offset + 0.0_s,
    offset + 1.0_s,
    offset + 2.0_s
  };
  Span<TimeStamp> span(offset, offset + 3.0_s);

  Arrayd dst{9, 7, 4};

  TemporalSplineCurve c(span, 1.0_s, src, dst);

  EXPECT_NEAR(c.evaluate(offset + 0.0_s), 9.0, 1.0e-4);
  EXPECT_NEAR(c.evaluate(offset + 1.0_s), 7.0, 1.0e-4);
  EXPECT_NEAR(c.evaluate(offset + 2.0_s), 4.0, 1.0e-4);
}
