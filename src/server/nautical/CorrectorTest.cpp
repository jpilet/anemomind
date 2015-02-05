/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/Corrector.h>
#include <gtest/gtest.h>
#include <server/common/Array.h>
#include <server/common/string.h>
#include <iostream>

using namespace sail;

static_assert(sizeof(double) == sizeof(AngleCorrector<double>), "It doesn't seem packed");
static_assert(4*sizeof(double) == sizeof(SpeedCorrector<double>), "It doesn't seem packed");
static_assert(sizeof(char) == sizeof(AngleCorrector<char>), "It doesn't seem packed");
static_assert(4*sizeof(char) == sizeof(SpeedCorrector<char>), "It doesn't seem packed");
static_assert(sizeof(short) == sizeof(AngleCorrector<short>), "It doesn't seem packed");
static_assert(4*sizeof(short) == sizeof(SpeedCorrector<short>), "It doesn't seem packed");

TEST(CorrectorTest, InitTest) {
  Corrector<double> corrector;
  Array<double> params = corrector.toArray();
  for (double p : params) {
    EXPECT_LT(std::abs(p), 1.0e2);
  }
}


TEST(CorrectorTest, NoCurrent) {
  Corrector<double> corrector;

  /*
   * A true wind blowing in the direction of south-west, with an angle of 225 degrees.
   * It is blowing from north-east with an angle of 45 degrees.
   */
  HorizontalMotion<double> trueWind =
      HorizontalMotion<double>::polar(Velocity<double>::knots(6), Angle<double>::degrees(225));

  /*
   * No current
   */
  HorizontalMotion<double> trueCurrent(Velocity<double>::knots(0), Velocity<double>::knots(0));

  // Suppose we are sailing downwind, slower than the wind.
  // We feel a small apparent wind in our back.
  class MeasuredData {
   public:
    Velocity<double> aws() const {return Velocity<double>::knots(1.5);}

    // Wind is from behind:
    Angle<double> awa() const {return Angle<double>::degrees(180);}

    Angle<double> magHdg() const {return Angle<double>::degrees(225);}
    Velocity<double> watSpeed() const {return Velocity<double>::knots(4.5);}

    Angle<double> gpsBearing() const {return Angle<double>::degrees(225);}
    Velocity<double> gpsSpeed() const {return Velocity<double>::knots(4.5);}
  };

  CalibratedNav<double> c = corrector.correct(MeasuredData());
  double marg = 1.0e-2;
  EXPECT_NEAR(c.trueWind()[0].knots(), trueWind[0].knots(), marg);
  EXPECT_NEAR(c.trueWind()[1].knots(), trueWind[1].knots(), marg);
  EXPECT_NEAR(c.trueCurrent()[0].knots(), trueCurrent[0].knots(), marg);
  EXPECT_NEAR(c.trueCurrent()[1].knots(), trueCurrent[1].knots(), marg);
}

TEST(CorrectorTest, BeamReachWithCurrent) {
  Corrector<double> corrector;

  // A wind of 12 knots blowing from east.
  HorizontalMotion<double> trueWind =
      HorizontalMotion<double>::polar(Velocity<double>::knots(12), Angle<double>::degrees(270));

  // A current of 1.3 knots coming from north and going south
  HorizontalMotion<double> trueCurrent =
      HorizontalMotion<double>::polar(Velocity<double>::knots(1.3), Angle<double>::degrees(180));

  // We sail north, against the current.
  class MeasuredData {
   public:
    Velocity<double> aws() const {return Velocity<double>::knots(12*sqrt(2));}

    // The true wind is coming from east, but since we
    // sail at the same speed as the wind is blowing
    // the wind appears to come slightly against us.
    Angle<double> awa() const {return Angle<double>::degrees(45);}

    Angle<double> magHdg() const {return Angle<double>::degrees(0);}
    Velocity<double> watSpeed() const {return Velocity<double>::knots(12 + 1.3);}

    Angle<double> gpsBearing() const {return Angle<double>::degrees(0);}
    Velocity<double> gpsSpeed() const {return Velocity<double>::knots(12);}
  };

  CalibratedNav<double> c = corrector.correct(MeasuredData());
  double marg = 1.0e-2;
  EXPECT_NEAR(c.trueWind()[0].knots(), trueWind[0].knots(), marg);
  EXPECT_NEAR(c.trueWind()[1].knots(), trueWind[1].knots(), marg);
  EXPECT_NEAR(c.trueCurrent()[0].knots(), trueCurrent[0].knots(), marg);
  EXPECT_NEAR(c.trueCurrent()[1].knots(), trueCurrent[1].knots(), marg);
  EXPECT_NEAR(c.twdir().degrees(), 90.0, marg);
}

/*// Try the corrector with FixPt numbers
TEST(CorrectorTest, FixPt) {

}*/


