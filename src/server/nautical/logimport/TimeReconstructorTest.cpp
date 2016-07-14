/*
 * TimeReconstructorTest.cpp
 *
 *  Created on: Jul 14, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/logimport/TimeReconstructor.h>

using namespace sail;

namespace {
  TimeStamp indexToTime(int index) {
    return TimeStamp::UTC(2016, 7, 13, 19, 22, 0) + double(index)*Duration<double>::seconds(2.4);
  }
}

TEST(LoggerTest, RegularizeTimeInPlace) {
  auto s = Duration<double>::seconds(1.0);


  int n = 12;
  std::vector<TimeStamp> times;
  for (int i = 0; i < n; i++) {
    times.push_back(indexToTime(i));
  }

  // Bad times:
  times[0] = TimeStamp::UTC(2070, 5, 4, 19, 22, 0);
  times[4] = TimeStamp::UTC(2048, 5, 4, 19, 22, 0);

  EXPECT_TRUE(regularizeTimesInPlace(&times));

  for (int i = 0; i < n; i++) {
    auto diff = times[i] - indexToTime(i);
    EXPECT_NEAR(diff.seconds(), 0.0, 0.001);
  }
}
