/*
 * IgnoreDataTest.cpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#include <server/nautical/timesets/IgnoreData.h>
#include <gtest/gtest.h>

using namespace sail;

TimeStamp t(double s) {
  return TimeStamp::UTC(2017, 9, 7, 16, 15, 0) + s*1.0_s;
}

TEST(IgnoreDataTest, TestIt) {
  auto d = std::make_shared<TypedDispatchDataReal<Angle<double>>>(
      AWA, "k", nullptr, 0);

  TimeSetInterval ivl;

  auto s = TimeSetTypes::ignoreButVisualize;
  /*Array<TimeSetInterval> intervals{
    {, Span<TimeStamp>(t(0), t(1))}
  };*/
}





