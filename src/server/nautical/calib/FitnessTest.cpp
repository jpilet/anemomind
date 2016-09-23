/*
 * FitnessTest.cpp
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

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

namespace {
  TimedValue<Angle<double> > tv(double x) {
    return TimedValue<Angle<double>>(
        TimeStamp::UTC(2016, 9, 23, 11, 1, 0),
        Angle<double>::degrees(x));
  }
}

TEST(FitnessTest, ValuesToFit) {
  typedef DistortionModel<double, AWA> Model;
  Model srcModel;
  double params[Model::paramCount];
  for (int i = 0; i < Model::paramCount; i++) {
    params[i] = i;
  }
  srcModel.readFrom(params);

  ValuesToFit<double, AWA> valuesToFit;
  valuesToFit.addValues(srcModel, {tv(1.4), tv(5.6), tv(6.7)});
  EXPECT_EQ(valuesToFit.distortionSpans.size(), 1);
  double params2[Model::paramCount];
  auto sp = valuesToFit.distortionSpans[0];
  sp.distortion.writeTo(params2);
  for (int i = 0; i < Model::paramCount; i++) {
    EXPECT_NEAR(params[i], params2[i], 1.0e-6);
  }
  EXPECT_EQ(sp.span.minv(), 0);
  EXPECT_EQ(sp.span.maxv(), 3);
  EXPECT_EQ(valuesToFit.values.size(), 3);
}

TEST(FitnessTest, DataFitTest) {

  BoatState<double> bs;
  Spani span(0,
      ReconstructedBoatState<double,
        FullSettings>::valueDimension);
  DataFit<double, FullSettings> fit(span, bs);


}
