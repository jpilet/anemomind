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

  double params[1] = {324.43};
  sensorSet.writeTo(params);
  EXPECT_EQ(params[0], 0.0);

  params[0] = 0.1;
  sensorSet.readFrom(params);

  sensorSet.outputSummary(&(std::cout));

  params[0] = 234324.324;
  sensorSet.writeTo(params);
  EXPECT_EQ(0.1, params[0]);

}


