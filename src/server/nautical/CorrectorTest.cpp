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

TEST(CalibModelTest, CountTest) {
  Corrector<double> corrector;
  static_assert(sizeof(double) == sizeof(AngleCorrector<double>), "It doesn't seem packed");
  static_assert(4*sizeof(double) == sizeof(SpeedCorrector<double>), "It doesn't seem packed");
  static_assert(sizeof(char) == sizeof(AngleCorrector<char>), "It doesn't seem packed");
  static_assert(4*sizeof(char) == sizeof(SpeedCorrector<char>), "It doesn't seem packed");
}


TEST(CalibModelTest, InitTest) {
  Corrector<double> corrector;
  Array<double> params = corrector.toArray();
  for (double p : params) {
    EXPECT_LT(std::abs(p), 1.0e2);
  }
}

namespace {
  std::ostream &operator<<(std::ostream &s, Velocity<double> x) {
    s << x.knots() << " knots";
    return s;
  }

  std::ostream &operator<<(std::ostream &s, Angle<double> x) {
    s << x.degrees() << " degrees";
    return s;
  }

  std::ostream &operator<<(std::ostream &s, HorizontalMotion<double> x) {
      s << "(x=" << x[0] << ", y=" << x[1] << ", dir=" << x.angle() << ", norm=" << x.norm() << ")";
      return s;
    }



  std::ostream &operator<<(std::ostream &s, CalibratedNav<double> n) {
    s << EXPR_AND_VAL_AS_STRING(n.gpsMotion.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.rawAwa.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.rawMagHdg.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.rawAws.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.rawWatSpeed.get()) << std::endl;

    // Values that need to be calibrated externally.
    s << EXPR_AND_VAL_AS_STRING(n. calibAwa.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.boatOrientation.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.calibAws.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.calibWatSpeed.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n. driftAngle.get()) << std::endl; // <-- Optional to calibrate.

    // Values that are populated using the fill(n..get()) method.
    // Depend on the calibrated values.
    s << EXPR_AND_VAL_AS_STRING(n.apparentWindAngleWrtEarth.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.apparentWind.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.trueWind.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.trueCurrent.get()) << std::endl;
    s << EXPR_AND_VAL_AS_STRING(n.boatMotionThroughWater.get()) << std::endl;
    return s;
  }
}

TEST(CalibModelTest, NoCurrent) {
  Corrector<double> corrector;

  /*
   * A true wind blowing in the direction of south-west, with an angle of 225 degrees.
   * It is blowing from north-east with an angle of 45 degrees.
   */
  HorizontalMotion<double> trueWind =
      HorizontalMotion<double>::polar(Velocity<double>::knots(6), Angle<double>::degrees(225));

  /*
   *  A true current of 0.5 knots in the direction of 315 (-45) degrees.
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
  EXPECT_NEAR(c.trueWind.get()[0].knots(), trueWind[0].knots(), marg);
  EXPECT_NEAR(c.trueWind.get()[1].knots(), trueWind[1].knots(), marg);
  EXPECT_NEAR(c.trueCurrent.get()[0].knots(), trueCurrent[0].knots(), marg);
  EXPECT_NEAR(c.trueCurrent.get()[1].knots(), trueCurrent[1].knots(), marg);
}

TEST(CalibModelTest, BeamReachWithCurrent) {
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
  EXPECT_NEAR(c.trueWind.get()[0].knots(), trueWind[0].knots(), marg);
  EXPECT_NEAR(c.trueWind.get()[1].knots(), trueWind[1].knots(), marg);
  EXPECT_NEAR(c.trueCurrent.get()[0].knots(), trueCurrent[0].knots(), marg);
  EXPECT_NEAR(c.trueCurrent.get()[1].knots(), trueCurrent[1].knots(), marg);
}



