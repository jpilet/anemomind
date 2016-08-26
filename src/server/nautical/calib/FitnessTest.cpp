/*
 * FitnessTest.cpp
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

#include <server/nautical/calib/Fitness.h>
#include <gtest/gtest.h>
#include <ceres/jet.h>

using namespace sail;

TEST(SensorTest, Various) {

  SensorSet<double> sensorSet;
  EXPECT_EQ(0, sensorSet.paramCount());
  sensorSet.AWA["NMEA2000asdfasdfas"] = SensorModel<double, AWA>();

  {
    EXPECT_EQ(1, sensorSet.paramCount());

    double params[1] = {324.43};
    sensorSet.writeTo(params);
    EXPECT_NEAR(params[0], 0.0, 1.0e-6);

    params[0] = 0.25;
    sensorSet.readFrom(params);
    params[0] = 234324.324;
    sensorSet.writeTo(params);
    EXPECT_NEAR(params[0], 0.25, 1.0e-6);
  }

  sensorSet.WAT_SPEED["NMEA0183Speedo"]
      = SensorModel<double, WAT_SPEED>();
  {
    EXPECT_EQ(2, sensorSet.paramCount());
    double params[2] = {0.0, 0.0};
    sensorSet.writeTo(params);
    EXPECT_NEAR(params[0], 0.25, 1.0e-6);
    EXPECT_NEAR(params[1], 1.0, 1.0e-6);

    SensorParameterMap<double> parameterMap;
    sensorSet.writeTo(&parameterMap);

    EXPECT_NEAR(0.25,
        parameterMap[AWA]["NMEA2000asdfasdfas"]["offset-radians"],
        1.0e-6);
    EXPECT_NEAR(1.0,
        parameterMap[WAT_SPEED]["NMEA0183Speedo"]["bias"],
        1.0e-6);

    parameterMap[AWA]["NMEA2000asdfasdfas"]["offset-radians"] = 119.34;
    parameterMap[WAT_SPEED]["NMEA0183Speedo"]["bias"] = 34.5;

    sensorSet.readFrom(parameterMap);
    sensorSet.writeTo(params);
    EXPECT_NEAR(params[0], 119.34, 1.0e-6);
    EXPECT_NEAR(params[1], 34.5, 1.0e-6);
  }


  auto sensorSet2 = sensorSet.cast<ceres::Jet<double, 4> >();
  {
    EXPECT_EQ(2, sensorSet2.paramCount());
    ceres::Jet<double, 4> params[2];
    sensorSet2.writeTo(params);
    EXPECT_NEAR(params[0].a, 119.34, 1.0e-6);
    EXPECT_NEAR(params[1].a, 34.5, 1.0e-6);
  }
}


