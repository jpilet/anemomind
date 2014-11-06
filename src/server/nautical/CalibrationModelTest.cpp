/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/CalibrationModel.h>
#include <gtest/gtest.h>
#include <server/common/Array.h>
#include <server/common/string.h>
#include <iostream>

using namespace sail;

TEST(CalibModelTest, CountTest) {
  CorrectorSet<double> set = CorrectorSet<double>::makeDefaultCorrectorSet();
  EXPECT_EQ(set.paramCount(), 1 + 1 + 4 + 4 + 2);
}


TEST(CalibModelTest, InitTest) {
  CorrectorSet<double> set = CorrectorSet<double>::makeDefaultCorrectorSet();
  Array<double> params = Array<double>::fill(set.paramCount(), 1.0e6);
  set.initialize(params.ptr());
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

TEST(CalibModelTest, NoCurrentTest) {
  CorrectorSet<double> set = CorrectorSet<double>::makeDefaultCorrectorSet();
  Array<double> params = set.makeInitialParams();


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

  CalibratedNav<double> c = set.calibrate(params.ptr(), MeasuredData());
  std::cout << EXPR_AND_VAL_AS_STRING(c) << std::endl;
  double marg = 1.0e-2;
  EXPECT_NEAR(c.trueCurrent.get()[0].knots(), 0.0, marg);
  EXPECT_NEAR(c.trueCurrent.get()[1].knots(), 0.0, marg);

  EXPECT_NEAR(c.trueWind.get()[0].knots(), -6/sqrt(2), marg);
  EXPECT_NEAR(c.trueWind.get()[1].knots(), -6/sqrt(2), marg);
}



