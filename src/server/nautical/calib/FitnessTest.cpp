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

using namespace sail;

struct TestSettings1 {
  static const bool withBoatOverGround = false;
  static const bool withCurrentOverGround = false;
  static const bool withHeel = false;
  static const bool withPitch = false;
};

static_assert(
    ReconstructedBoatState<double, TestSettings1>::valueDimension ==
        0 + 2 + 0 + 2 + 0 + 0, "Failed");

struct TestSettings2 {
  static const bool withBoatOverGround = true;
  static const bool withCurrentOverGround = false;
  static const bool withHeel = false;
  static const bool withPitch = false;
};

static_assert(
    ReconstructedBoatState<double, TestSettings2>::valueDimension ==
        2 + 2 + 0 + 2 + 0 + 0, "Failed");

struct TestSettings3 {
  static const bool withBoatOverGround = true;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = false;
  static const bool withPitch = false;
};

static_assert(
    ReconstructedBoatState<double, TestSettings3>::valueDimension ==
        2 + 2 + 2 + 2 + 0 + 0, "Failed");

struct TestSettings4 {
  static const bool withBoatOverGround = true;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = true;
  static const bool withPitch = false;
};

static_assert(
    ReconstructedBoatState<double, TestSettings4>::valueDimension ==
        2 + 2 + 2 + 2 + 1 + 0, "Failed");

struct TestSettings5 {
  static const bool withBoatOverGround = true;
  static const bool withCurrentOverGround = true;
  static const bool withHeel = true;
  static const bool withPitch = true;
};

static_assert(
    ReconstructedBoatState<double, TestSettings5>::valueDimension ==
        2 + 2 + 2 + 2 + 1 + 1, "Failed");

TEST(FitnessTest, ReconstructedBoatStateTest) {
  typedef ReconstructedBoatState<
      double, FullSettings> State;
  const int n = State::valueDimension;
  static_assert(0 < n, "It should be non-empty");
  {
    double values[n];
    for (int i = 0; i < n; i++) {
      values[i] = i;
    }

    State state;
    const double *src = values;
    state.read(&src);
    EXPECT_EQ(values + n, src);

    double values2[n];
    double *dst = values2;
    state.write(&dst);
    EXPECT_EQ(values2 + n, dst);

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
  ReconstructedBoatState<double, FullSettings> state;
  state.boatOverGround.value = HorizontalMotion<double>{4.0_kn, 0.0_kn};
  state.windOverGround.value = HorizontalMotion<double>{0.0_kn, -4.0_kn};
  state.heading.value = HorizontalMotion<double>{4.0_kn, 0.0_kn};

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
  }
}
