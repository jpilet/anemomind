/*
 * FitnessTest.cpp
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

#include <iostream>
#include <server/common/PhysicalQuantityIO.h>
#include <gtest/gtest.h>
#include <server/nautical/calib/Fitness.h>
#include <ceres/ceres.h>
#include <server/common/LineKM.h>

using namespace sail;

struct TestSettings1 {
  static const bool withBoatOverGround = false;
  static const bool withCurrentOverGround = false;
  static const bool withHeel = false;
  static const bool withPitch = false;
};

static_assert(
    ReconstructedBoatState<double, TestSettings1>::dynamicValueDimension ==
        0 + 2 + 0 + 2 + 0 + 0, "Failed");

struct TestSettings2 {
  static const bool withBoatOverGround = true;
  static const bool withCurrentOverGround = false;
  static const bool withHeel = false;
  static const bool withPitch = false;
};

static_assert(
    ReconstructedBoatState<double, TestSettings2>::dynamicValueDimension ==
        2 + 2 + 0 + 2 + 0 + 0, "Failed");

struct TestSettings3 {
  static const bool withBoatOverGround = true;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = false;
  static const bool withPitch = false;
};

static_assert(
    ReconstructedBoatState<double, TestSettings3>::dynamicValueDimension ==
        2 + 2 + 2 + 2 + 0 + 0, "Failed");

struct TestSettings4 {
  static const bool withBoatOverGround = true;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = true;
  static const bool withPitch = false;
};

static_assert(
    ReconstructedBoatState<double, TestSettings4>::dynamicValueDimension ==
        2 + 2 + 2 + 2 + 1 + 0, "Failed");

struct TestSettings5 {
  static const bool withBoatOverGround = true;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = true;
  static const bool withPitch = true;
};



static_assert(
    ReconstructedBoatState<double, TestSettings5>::dynamicValueDimension ==
        2 + 2 + 2 + 2 + 1 + 1, "Failed");

TEST(FitnessTest, ReconstructedBoatStateTest) {
  typedef ReconstructedBoatState<
      double, FullSettings> State;
  const int n = State::dynamicValueDimension;
  static_assert(0 < n, "It should be non-empty");
  {
    double values[n];
    for (int i = 0; i < n; i++) {
      values[i] = i;
    }

    State state;
    const double *src = values;
    state.readFrom(src);
    EXPECT_EQ(values, src);

    double values2[n];
    double *dst = values2;
    state.writeTo(dst);
    EXPECT_EQ(values2, dst);

    for (int i = 0; i < n; i++) {
      EXPECT_NEAR(values[i], values2[i], 1.0e-6);
    }
  }

  BoatState<double> bs;
  bs.setBoatOverGround(HorizontalMotion<double>(3.4_kn, 4.5_kn));
  auto state2 = State::make(bs);
  EXPECT_NEAR(state2.boatOverGround.value[0].knots(), 3.4, 1.0e-6);
  EXPECT_NEAR(state2.boatOverGround.value[1].knots(), 4.5, 1.0e-6);
}

TEST(FitnessTest, VectorizationTest) {
  double values[2] = {4.5, 5.6};
  const double *src = values;
  HorizontalMotion<double> x =
      TypeVectorizer<double, HorizontalMotion<double> >::read(&src);
  EXPECT_EQ(src, values + 2);
  double values2[2] = {0.0, 0.0};
  double *dst = values2;
  TypeVectorizer<double, HorizontalMotion<double> >::write(x, &dst);
  EXPECT_NEAR(values[0], values2[0], 1.0e-6);
  EXPECT_NEAR(values[1], values2[1], 1.0e-6);
}

TEST(FitnessTest, HuberTest) {
  EXPECT_NEAR(sqrtHuber<double>(0), 0.0, 1.0e-6);
  EXPECT_NEAR(sqrtHuber<double>(1), 1.0, 1.0e-6);
  EXPECT_NEAR(sqrtHuber<double>(2), sqrt(1 + 2), 1.0e-6);
  EXPECT_NEAR(sqrtHuber<double>(-2), -sqrt(1 + 2), 1.0e-6);
}

TEST(FitnessTest, ResidualTest) {
  HeelConstant<double> heelCoef = 2.3_deg/1.0_kn;

  ReconstructedBoatState<double, FullSettings> state;
  state.boatOverGround.value = HorizontalMotion<double>{4.0_kn, 0.0_kn};
  state.windOverGround.value = HorizontalMotion<double>{0.0_kn, -4.0_kn};
  state.heading.value = HorizontalMotion<double>{
    referenceVelocityForAngles<double>(), 0.0_kn};
  state.currentOverGround.value = HorizontalMotion<double>{-1.0_kn, 0.0_kn};
  state.heel.value = 4.0_kn*heelCoef;

  auto expectedAWA = -45.0_deg;
  auto expectedAWS = sqrt(2.0)*4.0_kn;

  // AWA tests
  DistortionModel<double, AWA> awaModel;
  {
    static_assert(1 == AWAFitness<double, FullSettings>::outputCount,
        "Not what we expected");
    double residuals[1] = {10.0};
    EXPECT_TRUE((AWAFitness<double, FullSettings>::apply(state, awaModel,
        expectedAWA, residuals)));
    EXPECT_NEAR(residuals[0], 0.0, 1.0e-6);

  }{
    double residuals[1] = {0.0};
    DistortionModel<double, AWA> bad;
    double awaParam = 234234.234234;
    bad.readFrom(&awaParam);
    EXPECT_TRUE((AWAFitness<double, FullSettings>::apply(state, bad,
        expectedAWA, residuals)));
    EXPECT_LT(0.001, std::abs(residuals[0]));
  }{
    double residuals[1] = {0.0};
        EXPECT_TRUE((AWAFitness<double, FullSettings>::apply(state, awaModel,
            20.0_deg, residuals)));
        EXPECT_LT(0.001, std::abs(residuals[0]));
  }{
    Angle<double> atBandWidth = Angle<double>::radians(
        BandWidth<double, AWA>::get()/expectedAWS);
    double residuals[1] = {0.0};
    EXPECT_TRUE((AWAFitness<double, FullSettings>::apply(state, awaModel,
      expectedAWA + atBandWidth, residuals)));
    EXPECT_NEAR(std::abs(residuals[0]), 1.0, 1.0e-3);
  }

  // AWS tests
  DistortionModel<double, AWS> awsModel;
  {
    static_assert(1 == AWSFitness<double, FullSettings>::outputCount,
        "Not what we expected");
    double residuals[1] = {10.0};
    EXPECT_TRUE((AWSFitness<double, FullSettings>::apply(
        state, awsModel, expectedAWS, residuals)));
    EXPECT_NEAR(residuals[0], 0.0, 1.0e-6);
  }{
    DistortionModel<double, AWS> bad;
    double params[2] = {34.4, 34.4};
    bad.readFrom(params);
    double residuals[1] = {0.0};
    EXPECT_TRUE((AWSFitness<double, FullSettings>::apply(
        state, bad, expectedAWS, residuals)));
    EXPECT_LT(0.001, std::abs(residuals[0]));
  }{
    double residuals[1] = {0.0};
    EXPECT_TRUE((AWSFitness<double, FullSettings>::apply(
        state, awsModel, 30.0_kn, residuals)));
    EXPECT_LT(0.1, std::abs(residuals[0]));
  }{
    double residuals[1] = {0.0};
    EXPECT_TRUE((AWSFitness<double, FullSettings>::apply(
        state, awsModel, expectedAWS +
          BandWidth<double, AWS>::get(), residuals)));
    EXPECT_NEAR(1.0, std::abs(residuals[0]), 1.0e-6);
  }

  Angle<double> expectedHeading = 90.0_deg;

  // MAG_HEADING tests
  DistortionModel<double, MAG_HEADING> hdgModel;
  {
    static_assert(1 == MagHeadingFitness<double,
        FullSettings>::outputCount,
        "Not what we expected");
    double residuals[1] = {10.0};
    EXPECT_TRUE((MagHeadingFitness<double, FullSettings>::apply(
        state, hdgModel, expectedHeading, residuals)));
    EXPECT_NEAR(residuals[0], 0.0, 1.0e-6);
  }{
    double residuals[1] = {0.0};
    DistortionModel<double, MAG_HEADING> bad;
    double params[1] = {6435.543};
    bad.readFrom(params);
    EXPECT_TRUE((MagHeadingFitness<double, FullSettings>::apply(
        state, bad, expectedHeading, residuals)));
    EXPECT_LT(0.001, std::abs(residuals[0]));
  }{
    double residuals[1] = {0.0};
    EXPECT_TRUE((MagHeadingFitness<double, FullSettings>::apply(
        state, hdgModel, 180.0_deg, residuals)));
    EXPECT_LT(0.001, std::abs(residuals[0]));
  }{
    double residuals[1] = {0.0};
    EXPECT_TRUE((MagHeadingFitness<double, FullSettings>::apply(
        state, hdgModel, expectedHeading +
        BandWidth<double, MAG_HEADING>::get(), residuals)));
    EXPECT_NEAR(1.0, std::abs(residuals[0]), 1.0e-3);
  }

  // WAT_SPEED tests
  auto expectedWatSpeed = 5.0_kn;
  DistortionModel<double, WAT_SPEED> watModel;
  {
    static_assert(1 == MagHeadingFitness<double,
        FullSettings>::outputCount, "Bad");
    double residuals[1] = {10.0};
    EXPECT_TRUE((WatSpeedFitness<double, FullSettings>::apply(
        state, watModel, expectedWatSpeed, residuals)));
    EXPECT_NEAR(0.0, residuals[0], 1.0e-6);
  }{
    double residuals[1] = {0.0};
    DistortionModel<double, WAT_SPEED> bad;
    double ws[2] = {34.4, 5.6};
    bad.readFrom(ws);
    EXPECT_TRUE((WatSpeedFitness<double, FullSettings>::apply(
        state, bad, expectedWatSpeed, residuals)));
    EXPECT_LT(0.01, std::abs(residuals[0]));
  }{
    double residuals[1] = {0.0};
    EXPECT_TRUE((WatSpeedFitness<double, FullSettings>::apply(
        state, watModel, 90.0_kn, residuals)));
    EXPECT_LT(0.01, std::abs(residuals[0]));
  }{
    double residuals[1] = {0.0};
    EXPECT_TRUE((WatSpeedFitness<double, FullSettings>::apply(
        state, watModel, expectedWatSpeed
        + BandWidth<double, WAT_SPEED>::get(), residuals)));
    EXPECT_NEAR(1.0, std::abs(residuals[0]), 1.0e-6);
  }

  // Heel angle tests
  {
    double residuals[1] = {10.0};
    EXPECT_TRUE((HeelFitness<double, FullSettings>::apply(
        state, heelCoef, residuals)));
    EXPECT_NEAR(residuals[0], 0.0, 1.0e-6);
  }{
    state.heel.value += HeelFitness<double, FullSettings>::bandWidth();
    double residuals[1] = {10.0};
    EXPECT_TRUE((HeelFitness<double, FullSettings>::apply(
        state, heelCoef, residuals)));
    EXPECT_NEAR(std::abs(residuals[0]), 1.0, 1.0e-6);
  }

  // Leeway angle tests
  LeewayConstant<double> leewayConstant = 1.0_kn*1.0_kn;
  {
    double residuals[1] = {10.0};
    state.heel.value = Angle<double>::degrees(0.0);
    EXPECT_TRUE(
        (LeewayFitness<double, FullSettings>::apply(state, leewayConstant,
        residuals)));
    EXPECT_NEAR(residuals[0], 0.0, 1.0e-6);
  }{
    auto old = state.heading;
    state.heading.value = state.heading.value.rotate(5.0_deg);
    double residuals[1] = {0.0};
    EXPECT_TRUE(
        (LeewayFitness<double, FullSettings>::apply(state, leewayConstant,
        residuals)));
    EXPECT_LT(0.01, std::abs(residuals[0]));
    state.heading = old;
  }{
    state.heel.value = 12.0_deg;
    double residuals[1] = {0.0};
    EXPECT_TRUE(
        (LeewayFitness<double, FullSettings>::apply(state, leewayConstant,
        residuals)));
    EXPECT_LT(0.01, std::abs(residuals[0]));
    state.heel.value = 0.0_deg;
  }


}

TEST(FitnessTest, OrientationTest) {
  ReconstructedBoatState<double, FullSettings> state;

  state.heading.value = HorizontalMotion<double>::polar(
      referenceVelocityForAngles<double>(), 0.0_deg);

  {
    DistortionModel<double, ORIENT> model;
    auto observed = AbsoluteOrientation{0.0_deg, 0.0_deg, 0.0_deg};
    double r[1] = {10.0};
    EXPECT_TRUE((OrientFitness<double, FullSettings>::apply(
        state, model, observed, r)));
    EXPECT_NEAR(r[0], 0.0, 1.0e-3);
  }{
    DistortionModel<double, ORIENT> model;
    auto observed = AbsoluteOrientation{0.0_deg, 0.0_deg,
      BandWidthForType<double, Angle<double>>::get()};
    double r[1] = {10.0};
    EXPECT_TRUE((OrientFitness<double, FullSettings>::apply(
        state, model, observed, r)));
    EXPECT_NEAR(r[0], 1.0, 1.0e-3);
  }

  state.heading.value = HorizontalMotion<double>::polar(
      referenceVelocityForAngles<double>()
      - BandWidthForType<double, Velocity<double>>::get(), 0.0_deg);
  {
    DistortionModel<double, ORIENT> model;
    auto observed = AbsoluteOrientation{0.0_deg, 0.0_deg, 0.0_deg};
    double r[1] = {10.0};
    EXPECT_TRUE((OrientFitness<double, FullSettings>::apply(
        state, model, observed, r)));
    EXPECT_NEAR(r[0], 1.0, 1.0e-3);
  }

  state.heading.value = HorizontalMotion<double>::polar(
      referenceVelocityForAngles<double>(), 0.0_deg);
  {
    DistortionModel<double, ORIENT> model;
    double params[3] = {1.0, 3.4, 99.34};
    model.readFrom(params);
    auto observed = AbsoluteOrientation{0.0_deg, 0.0_deg, 0.0_deg};
    double r[1] = {0.0};
    EXPECT_TRUE((OrientFitness<double, FullSettings>::apply(
        state, model, observed, r)));
    EXPECT_LT(0.001, r[0]);
  }{
    DistortionModel<double, ORIENT> model;
    double params[3] = {0.0, 0.0,
        BandWidth<double, ORIENT>::get().radians()};
    model.readFrom(params);
    auto observed = AbsoluteOrientation{0.0_deg, 0.0_deg, 0.0_deg};
    double r[1] = {0.0};
    EXPECT_TRUE((OrientFitness<double, FullSettings>::apply(
        state, model, observed, r)));
    EXPECT_NEAR(r[0], 1.0, 1.0e-3);
  }
}



bool eqMotion(const HorizontalMotion<double> &a,
              const HorizontalMotion<double> &b) {
  return HorizontalMotion<double>(a - b).norm().knots() < 1.0e-4;
}


TEST(FitnessTest, Convert) {
  ReconstructedBoatState<double, DefaultSettings> bs;
  bs.boatOverGround.value = HorizontalMotion<double>{1.0_kn, 2.0_kn};
  bs.windOverGround.value = HorizontalMotion<double>{3.0_kn, 4.0_kn};
  bs.currentOverGround.value = HorizontalMotion<double>{5.0_kn, 6.0_kn};
  bs.heading.value = HorizontalMotion<double>::polar(3.0_kn, 94.0_deg);
  bs.heel.value = 30.0_deg;
  bs.pitch.value = 15.0_deg;

  BoatState<double> bs0;

  BoatState<double> bs2 = bs.makeBoatState(bs0);
  EXPECT_TRUE(eqMotion(bs2.boatOverGround(), bs.boatOverGround.value));
  EXPECT_TRUE(eqMotion(bs2.windOverGround(), bs.windOverGround.value));
  EXPECT_TRUE(eqMotion(bs2.currentOverGround(), bs.currentOverGround.value));
  EXPECT_NEAR((bs2.heading() - bs.heading.value.angle())
      .normalizedAt0().degrees(), 0.0, 1.0e-6);
}
