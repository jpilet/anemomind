/*
 * CorrectionTest.cpp
 *
 *  Created on: Apr 29, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/calib/Correction.h>

using namespace sail;
using namespace sail::Correction;

namespace {
  auto deg = Angle<double>::degrees(1.0);
  auto kt = Velocity<double>::knots(1.0);

}

TEST(CorrectionTest, DriftUpwind) {
  EXPECT_NEAR(upwindTwaDriftWeighting(-0.5*M_PI), 0.0, 1.0e-6);
  EXPECT_NEAR(upwindTwaDriftWeighting(0.0*M_PI), 0.0, 1.0e-6);
  EXPECT_NEAR(upwindTwaDriftWeighting(0.5*M_PI), 0.0, 1.0e-6);


  auto h = 1.0e-8;
  EXPECT_NEAR(
      (0.5/h)*(upwindTwaDriftWeighting(0.5*M_PI + h)
          - upwindTwaDriftWeighting(0.5*M_PI - h)),
      0.0, 1.0e-6);
  EXPECT_NEAR(
      (0.5/h)*(upwindTwaDriftWeighting(-0.5*M_PI + h)
          - upwindTwaDriftWeighting(-0.5*M_PI - h)),
      0.0, 1.0e-6);

  auto middleSlope = (0.5/h)*(upwindTwaDriftWeighting(h)
                            - upwindTwaDriftWeighting(-h));


  EXPECT_LT(0.01, middleSlope);
}

TEST(CorrectionTest, GeneralDrift) {

  EXPECT_NEAR(twaDriftWeighting<double>(0.0*deg), 0.0, 1.0e-6);
  EXPECT_NEAR(twaDriftWeighting<double>(90.0*deg), 0.0, 1.0e-6);
  EXPECT_NEAR(twaDriftWeighting<double>(-90.0*deg), 0.0, 1.0e-6);
  EXPECT_NEAR(twaDriftWeighting<double>(360.0*deg), 0.0, 1.0e-6);
  EXPECT_NEAR(twaDriftWeighting<double>(720.0*deg), 0.0, 1.0e-6);
  EXPECT_LT(0.01, twaDriftWeighting<double>(45.0*deg));
  EXPECT_LT(twaDriftWeighting<double>(-45.0*deg), -0.01);

  EXPECT_NEAR(
        -twaDriftWeighting<double>(45.0*deg),
        twaDriftWeighting<double>(-45.0*deg), 1.0e-6);

  EXPECT_NEAR(
        twaDriftWeighting<double>(45.0*deg),
        twaDriftWeighting<double>((45.0 + 360.0)*deg), 1.0e-6);

  EXPECT_NEAR(twaDriftWeighting<double>(100.0*deg), 0.0, 1.0e-6);
  EXPECT_NEAR(twaDriftWeighting<double>(120.0*deg), 0.0, 1.0e-6);
  EXPECT_NEAR(twaDriftWeighting<double>(200.0*deg), 0.0, 1.0e-6);
  EXPECT_NEAR(twaDriftWeighting<double>(560.0*deg), 0.0, 1.0e-6);
}

TEST(CorrectionTest, FullBasicCorrectorOnlyGpsMotion) {

  BasicCorrectorParams<double> params;

  RawSample sample;
  sample.gpsBearing = 119.0*deg;
  sample.gpsSpeed = 3.4*kt;

  sample.watSpeed = 0.0*kt;
  sample.magHeading = 0.0*deg;
  sample.awa = 0.0*deg;
  sample.aws = 0.0*kt;

  auto boatMotion = sample.gpsMotion();

  BasicFullCorrector corr;
  auto output = corr.apply(reinterpret_cast<double *>(&params), sample);

  auto estWind = output[0];
  auto estCurrent = output[1];

  // Since we register no apparent wind, we expect the wind over ground to be
  // the same as the GPS motion
  EXPECT_NEAR(estWind[0].knots(), boatMotion[0].knots(), 1.0e-6);
  EXPECT_NEAR(estWind[1].knots(), boatMotion[1].knots(), 1.0e-6);

  // And since we register no apparent current, we expect the current to be
  // the same as the GPS motion
  EXPECT_NEAR(estCurrent[0].knots(), boatMotion[0].knots(), 1.0e-6);
  EXPECT_NEAR(estCurrent[1].knots(), boatMotion[1].knots(), 1.0e-6);
}


TEST(CorrectionTest, FullBasicCorrectorStationary) {
  RawSample sample;
  sample.gpsBearing = 0.0*deg;
  sample.gpsSpeed = 0.0*kt;

  sample.watSpeed = 1.2*kt;
  sample.magHeading = 17.0*deg;
  sample.awa = -89.0*deg;
  sample.aws = 7.3*kt;

  BasicCorrectorParams<double> params;

  BasicFullCorrector corr;
  auto output = corr.apply(reinterpret_cast<double *>(&params), sample);

  auto estWind = output[0];
  auto estCurrent = output[1];

  auto expectedWind = HorizontalMotion<double>::polar(7.3*kt, 108.0*deg);
  EXPECT_NEAR(expectedWind[0].knots(), estWind[0].knots(), 1.0e-6);
  EXPECT_NEAR(expectedWind[1].knots(), estWind[1].knots(), 1.0e-6);

  auto expectedCurrent = HorizontalMotion<double>::polar(1.2*kt, 197.0*deg);
  EXPECT_NEAR(expectedCurrent[0].knots(), estCurrent[0].knots(), 1.0e-6);
  EXPECT_NEAR(expectedCurrent[1].knots(), estCurrent[1].knots(), 1.0e-6);
}
