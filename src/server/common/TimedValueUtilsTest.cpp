/*
 * TimedValueUtilsTest.cpp
 *
 *  Created on: 10 Nov 2016
 *      Author: jonas
 */

#include <server/common/TimedValueUtils.h>
#include <gtest/gtest.h>
#include <server/common/ArrayIO.h>

using namespace sail;

namespace {
  auto offset = TimeStamp::UTC(2016, 11, 10, 17, 50, 0);

  TimeStamp t(double s) {
    return offset + s*1.0_s;
  }
}

TEST(TimedValueUtils, TestIt) {
  auto bds = listAllBounds({t(0), t(1), t(2), t(3), t(9)}, 2.0_s);
  EXPECT_EQ((Array<int>{0, 4, 5}), bds);
  EXPECT_EQ((Array<int>{0}), (listAllBounds({}, 2.0_s)));
}
