/*
 *  Created on: 2014-10-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <device/Arduino/libraries/TargetSpeed/PolarSpeedTable.h>
#include <string>
#include <server/common/Env.h>
#include <gtest/gtest.h>
#include <cmath>

using namespace sail;

namespace {
  typedef PolarSpeedTable::FixType FixType;

  std::string getTempFilename() {
    return std::string(Env::BINARY_DIR) + "/temp_polar_speed_table.dat";
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



TEST(PolarSpeedTableTest, NonEmptyTest) {
  SpeedLookUp lu;
  Velocity<double> twsStep = Velocity<double>::knots(1.0);
  int twsCount = 30;
  int twaCount = 12;
  Angle<double> twaStep = Angle<double>::degrees(30.0);
  std::string filename = getTempFilename();
  { // Build it
    EXPECT_TRUE(PolarSpeedTable::build(twsStep,
        twsCount, twaCount, lu, filename.c_str()));
  }{
    PolarSpeedTable table(filename.c_str());
    EXPECT_FALSE(table.empty());


    {
      double tws = 0.0;
      double twa = 0.0;

      double luValue = double(table.targetSpeed(Velocity<FixType>::knots(FixType(tws)),
                                                   Angle<FixType>::degrees(FixType(twa))).knots());
      double gtValue = lu(Velocity<double>::knots(tws), Angle<double>::degrees(twa)).knots();
      EXPECT_NEAR(luValue, gtValue, 0.01);
    }
  }
}

