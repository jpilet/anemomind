/*
 * EcefTest.cpp
 *
 *  Created on: 30 Oct 2016
 *      Author: jonas
 */

#include <server/nautical/Ecef.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(EcefTest, BasicBackAndForth) {
  auto lon = 34.0_deg;
  auto lat = 49.0_deg;
  auto h = 5.7_m;
  LLACoords<double> src{
    lon, lat, h
  };

  auto dst = ECEF::convert(src);
  auto src2 = ECEF::convert(dst);

  EXPECT_NEAR(cos(src.lat), cos(src2.lat), 1.0e-6);
  EXPECT_NEAR(sin(src.lat), sin(src2.lat), 1.0e-6);
  EXPECT_NEAR(cos(src.lon), cos(src2.lon), 1.0e-6);
  EXPECT_NEAR(sin(src.lon), sin(src2.lon), 1.0e-6);
  EXPECT_NEAR(src.height.meters(), src2.height.meters(), 1.0e-6);
}



