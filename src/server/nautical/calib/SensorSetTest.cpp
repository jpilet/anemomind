/*
 * SensorSetTest.cpp
 *
 *  Created on: 1 Sep 2016
 *      Author: jonas
 */

#include <server/nautical/calib/SensorSet.h>
#include <gtest/gtest.h>
#include <ceres/jet.h>
#include <random>
#include <ceres/ceres.h>

using namespace sail;

TEST(SensorTest, Various) {

  SensorSet<double> sensorSet;
  EXPECT_EQ(0, sensorSet.paramCount());
  sensorSet.AWA["NMEA2000asdfasdfas"] = SensorModel<double, AWA>();

  {
    EXPECT_EQ(2, sensorSet.paramCount());

    double params[2] = {324.43, 5.6};
    sensorSet.writeTo(params);
    EXPECT_NEAR(params[0], 0.0, 1.0e-6);
    EXPECT_NEAR(params[1], 0.0, 1.0e-6);

      params[0] = 0.25;
      sensorSet.readFrom(params);
      params[0] = 234324.324;
      sensorSet.writeTo(params);
      EXPECT_NEAR(params[0], 0.25, 1.0e-6);
  }

  sensorSet.WAT_SPEED["NMEA0183Speedo"]
      = SensorModel<double, WAT_SPEED>();
  {
    EXPECT_EQ(4, sensorSet.paramCount());
    double params[4] = {0.0, 0.0, 0.0, 0.0};
    sensorSet.writeTo(params);
    EXPECT_NEAR(params[0], 0.25, 1.0e-6);
    EXPECT_NEAR(params[2], 1.0, 1.0e-6);

    SensorParameterMap<double> parameterMap;
    sensorSet.writeTo(&parameterMap);

    EXPECT_NEAR(0.25,
        parameterMap[AWA]["NMEA2000asdfasdfas"]["dist"]["offset-radians"],
        1.0e-6);
    EXPECT_NEAR(1.0,
        parameterMap[WAT_SPEED]["NMEA0183Speedo"]["dist"]["bias"],
        1.0e-6);

    parameterMap[AWA]["NMEA2000asdfasdfas"]["dist"]["offset-radians"] = 119.34;
    parameterMap[WAT_SPEED]["NMEA0183Speedo"]["dist"]["bias"] = 34.5;

    sensorSet.readFrom(parameterMap);
    sensorSet.writeTo(params);
    EXPECT_NEAR(params[0], 119.34, 1.0e-6);
    EXPECT_NEAR(params[2], 34.5, 1.0e-6);
  }
  auto sensorSet2 = sensorSet.cast<ceres::Jet<double, 4> >();
  {
    EXPECT_EQ(4, sensorSet2.paramCount());
    ceres::Jet<double, 4> params[4];
    sensorSet2.writeTo(params);
    EXPECT_NEAR(params[0].a, 119.34, 1.0e-6);
    EXPECT_NEAR(params[2].a, 34.5, 1.0e-6);
  }
}

namespace {
  struct VelocityFit {
    Velocity<double> src, dst;

    template <typename T>
      bool operator()(const T* const x,
                      T* residuals) const {
      SensorModel<T, AWS> model;
      model.readFrom(x);
      Velocity<T> error = model.dist.apply(src) - dst;
      residuals[0] = sqrt(1.0e-9 + model.noiseCost.apply(error));
      return true;
    }
  };
}

TEST(SensorTest, BasicFit) {

  double k = 1.3945;

  std::default_random_engine rng;
  auto xUnit = 1.0_kn;
  std::uniform_real_distribution<double> Xdistrib(0, 12);
  std::uniform_real_distribution<double> noise(-0.3, 0.3);
  std::uniform_real_distribution<double> outlierDistrib(-20, 20);

  const int inlierCount = 30;
  const int outlierCount = 7;
  const int count = inlierCount + outlierCount;
  Velocity<double> X[count];
  Velocity<double> Y[count];
  for (int i = 0; i < count; i++) {
    auto x = Xdistrib(rng)*xUnit;
    X[i] = x;
    Y[i] = i < inlierCount?
        k*x : outlierDistrib(rng)*xUnit;
  }

  SensorModel<double, AWS> model;
  const int N = SensorModel<double, AWS>::paramCount;
  double params[N];
  model.writeTo(params);

  ceres::Problem problem;
  problem.AddParameterBlock(params, N);
  for (int i = 0; i < count; i++) {
    problem.AddResidualBlock(
        new ceres::AutoDiffCostFunction<VelocityFit, 1, N>(
            new VelocityFit{X[i], Y[i]}), nullptr, params);
  }
  ceres::Solver::Options options;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  model.readFrom(params);
  model.outputSummary(&(std::cout));

}


