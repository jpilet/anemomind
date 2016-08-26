/*
 * FitnessTest.cpp
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

#include <server/nautical/calib/Fitness.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(SensorTest, Instantiate) {

  SensorSet<double> sensorSet;
  EXPECT_EQ(0, sensorSet.paramCount());
  sensorSet.AWA["NMEA2000asdfasdfas"] = SensorModel<double, AWA>();
  EXPECT_EQ(1, sensorSet.paramCount());
}


