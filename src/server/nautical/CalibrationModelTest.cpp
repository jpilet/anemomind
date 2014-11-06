/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/CalibrationModel.h>
#include <gtest/gtest.h>
#include <server/common/Array.h>

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

TEST(CalibModelTest, NoCurrentTest) {
  CorrectorSet<double> set = CorrectorSet<double>::makeDefaultCorrectorSet();
  Array<double> params = Array<double>::fill(set.paramCount(), 1.0e6);

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
    Angle<double> awa() const {return Angle<double>::degrees(45);}

    Angle<double> magHdg() const {return Angle<double>::degrees(225);}
    Velocity<double> watSpeed() const {return Velocity<double>::knots(4.5);}

    Angle<double> gpsBearing() const {return Angle<double>::degrees(225);}
    Velocity<double> gpsSpeed() const {return Velocity<double>::knots(4.5);}
  };

  CalibratedNav<double> c = set.calibrate(params.ptr(), MeasuredData());
  double marg = 1.0e-2;
  EXPECT_NEAR(c.trueCurrent.get()[0].knots(), 0.0, marg);
  EXPECT_NEAR(c.trueCurrent.get()[1].knots(), 0.0, marg);

}



