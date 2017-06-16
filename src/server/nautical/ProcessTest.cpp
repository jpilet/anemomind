/*
 * ProcessTest.cpp
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/Process.h>
#include <server/nautical/InvWGS84.h>

using namespace sail;

auto o = TimeStamp::UTC(2017, 7, 30, 18, 40, 0);

TEST(ProcessTest, AdjustTimeTest) {
  LogFileInfo d0, d1, d2, d3;
  d0.medianTime = o;
  d0.maxTime = o + 2.0_s;

  d1.minTime = o + 3.0_s;
  d1.medianTime = o + 4.0_s;

  d2.minTime = o + 1.0_s;
  d2.medianTime = o + 3.0_s;

  d3.minTime = o - 8.0_s;
  d3.medianTime = o + 1.0_s;

  {
    auto a = adjustBoundaries(std::vector<LogFileInfo>{d0, d1});
    EXPECT_EQ(a[0].maxTime, o + 2.0_s);
    EXPECT_EQ(a[1].minTime, o + 3.0_s);
  }{
    auto a = adjustBoundaries(std::vector<LogFileInfo>{d0, d2});
    EXPECT_EQ(a[0].maxTime, o + 1.5_s);
    EXPECT_EQ(a[1].minTime, o + 1.5_s);
  }{
    auto a = adjustBoundaries(std::vector<LogFileInfo>{d0, d3});
    EXPECT_EQ(a[0].maxTime, o);
    EXPECT_EQ(a[1].minTime, o);
  }
}



GeographicPosition<double> local2gps(const Eigen::Vector2d& u) {
  Length<double> xyz[3] = {u(0)*1.0_m, 6366197.0_m, u(1)*1.0_m};
  return computeGeographicPositionFromXYZ(xyz);
}

TimeStamp t(double x) {
  return TimeStamp::UTC(2017, 8, 4, 17, 1, 0) + x*1.0_s;
}

TimedValue<GeographicPosition<double>> testData1(double x) {
  return TimedValue<GeographicPosition<double>>(
      t(0.3*x), local2gps(Eigen::Vector2d(x*3.4, x*9.8)));
}

TimedValue<GeographicPosition<double>> testData2(double x) {
  return TimedValue<GeographicPosition<double>>(
      t(0.2*x), local2gps(Eigen::Vector2d(x*5.4, 3000 - x*0.1)));
}
