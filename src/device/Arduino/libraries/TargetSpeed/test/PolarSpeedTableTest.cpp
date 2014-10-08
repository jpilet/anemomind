/*
 *  Created on: 2014-10-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <sstream>
#include <device/Arduino/libraries/TargetSpeed/PolarSpeedTable.h>
#include <string>
#include <server/common/Env.h>
#include <gtest/gtest.h>
#include <cmath>

using namespace sail;

MockSD SD;

namespace {
  typedef PolarSpeedTable::FixType FixType;

  const char *getTempFilename() {
    return "polar_speed_table.h";
  }

  class SpeedLookUp {
   public:
    Velocity<double> operator() (Velocity<double> tws, Angle<double> twa) {
      return 0.8*tws*sin(0.5*twa.radians());
    }
  };
}

TEST(PolarSpeedTableTest, EmptyTest) {
  PolarSpeedTable table;
  EXPECT_TRUE(table.empty());
}

void makeTable(Velocity<double> twsStep, int twsCount, int twaCount, SpeedLookUp lu) {
  std::stringstream ss;
  EXPECT_TRUE(PolarSpeedTable::build(twsStep,
          twsCount, twaCount, lu, &ss));
  SD.setReadableFile(std::string(getTempFilename()), ss.str());
}


TEST(PolarSpeedTableTest, NonEmptyTest) {
  SpeedLookUp lu;
  Velocity<double> twsStep = Velocity<double>::knots(1.0);
  int twsCount = 30;
  int twaCount = 12;
  makeTable(twsStep, twsCount, twaCount, lu);
  {
    PolarSpeedTable table;
    EXPECT_TRUE(table.load(getTempFilename()));
    EXPECT_FALSE(table.empty());

    const int testCount = 3;
    double TWS[testCount] = {0.0, 3.0, 4.0};
    double TWA[testCount] = {0.0, 30.0, 90.0};

    for (int i = 0; i < testCount; i++) {
      double tws = TWS[i];
      double twa = TWA[i];

      double luValue = double(table.targetSpeed(Velocity<FixType>::knots(FixType(tws)),
                                                   Angle<FixType>::degrees(FixType(twa))).knots());
      double gtValue = lu(Velocity<double>::knots(tws), Angle<double>::degrees(twa)).knots();
      EXPECT_NEAR(luValue, gtValue, 0.01);
    }
  }
}

TEST(PolarSpeedTableTest, Interpolation) {
  SpeedLookUp lu;
  Velocity<double> twsStep = Velocity<double>::knots(1.0);
  int twsCount = 30;
  int twaCount = 12;
  makeTable(twsStep, twsCount, twaCount, lu);
  {
    PolarSpeedTable table;
    EXPECT_TRUE(table.load(getTempFilename()));
    EXPECT_FALSE(table.empty());

    const int testCount = 5;
    double TWS[testCount] = {3.0, 3.0, 4.0, 4.0,         3.25};
    double TWA[testCount] = {30.0, 60.0, 30.0, 60.0,     45.0};
    double res[testCount];

    for (int i = 0; i < testCount; i++) {
      double tws = TWS[i];
      double twa = TWA[i];

      res[i] = double(table.targetSpeed(Velocity<FixType>::knots(FixType(tws)),
                                                   Angle<FixType>::degrees(FixType(twa))).knots());
    }
    EXPECT_NEAR(res[4], 0.75*(0.5*res[0] + 0.5*res[1]) + 0.25*(0.5*res[2] + 0.5*res[3]), 0.1);
  }
}

TEST(PolarSpeedTableTest, Cyclic) {
  SpeedLookUp lu;
  Velocity<double> twsStep = Velocity<double>::knots(1.0);
  int twsCount = 30;
  int twaCount = 12;
  Angle<double> twaStep = Angle<double>::degrees(30.0);
  makeTable(twsStep, twsCount, twaCount, lu);
  {
    PolarSpeedTable table;
    EXPECT_TRUE(table.load(getTempFilename()));
    EXPECT_FALSE(table.empty());

    const int testCount = 2;
    double TWS[testCount] = {3.0, 3.0};
    double TWA[testCount] = {30.0, 30.0 + 720.0};
    double res[testCount];

    for (int i = 0; i < testCount; i++) {
      double tws = TWS[i];
      double twa = TWA[i];

      res[i] = double(table.targetSpeed(Velocity<FixType>::knots(FixType(tws)),
                                                   Angle<FixType>::degrees(FixType(twa))).knots());
    }
    EXPECT_NEAR(res[0], res[1], 1.0e-6);
  }
}

