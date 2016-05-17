/*
 * CeresTrajectoryFilter.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <server/math/nonlinear/CeresTrajectoryFilter.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {
  typedef TimedObservation<1> Obs1;
  Obs1 obs(int order, double time, double x) {
    Eigen::Matrix<double, 1, 1> v;
    v(0) = x;
    return Obs1{order, time, v};
  }
}

using namespace CeresTrajectoryFilter;

TEST(FilterTest, TestNoOutliersPositions) {


  Array<Obs1> observations{
    obs(0, 0.0, 3.4),
    obs(0, 3.0, 5.5)
  };

  Settings settings;
  settings.regWeight = 0.0;
  settings.ceresOptions.minimizer_progress_to_stdout = false;
  auto Y = filter(observations, settings, Array<Obs1::Vec>());

  EXPECT_EQ(Y.size(), 2);

  EXPECT_NEAR(Y[0](0), 3.4, 1.0e-6);
  EXPECT_NEAR(Y[1](0), 5.5, 1.0e-6);
}

TEST(FilterTest, TestBoundaryFit) {


  Array<Obs1> observations{
    obs(0, 1.0, 3.4),
    obs(1, 3.0, 1.1) // <-- Should have position 3.4 + (3 - 1)*1.1 = 3.4 + 2.2 = 5.6
  };

  Settings settings;
  settings.regWeight = 0.0;
  settings.ceresOptions.minimizer_progress_to_stdout = false;
  auto Y = filter(observations, settings, Array<Obs1::Vec>());

  EXPECT_EQ(Y.size(), 2);

  EXPECT_NEAR(Y[0](0), 3.4, 1.0e-6);
  EXPECT_NEAR(Y[1](0), 5.6, 1.0e-6);
}

TEST(FilterTest, TestBoundaryFit2) {


  Array<Obs1> observations{
    obs(1, 1.0, 1.1),
    obs(0, 3.0, 5.6)
  };

  Settings settings;
  settings.regWeight = 0.0;
  settings.ceresOptions.minimizer_progress_to_stdout = false;
  auto Y = filter(observations, settings, Array<Obs1::Vec>());

  EXPECT_EQ(Y.size(), 2);

  EXPECT_NEAR(Y[0](0), 3.4, 1.0e-6);
  EXPECT_NEAR(Y[1](0), 5.6, 1.0e-6);
}

TEST(FilterTest, TestInnerDerivativeFit) {

  Array<Obs1> observations{
    obs(1, 1.0, 1.1), // 3.4 - 1.1*2 = 3.4 - 2.2 = 1.2
    obs(1, 3.0, 1.1), // 5.6 - 1.1*2 = 5.6 - 2.2 = 3.4
    obs(0, 5.0, 5.6)
  };

  Settings settings;
  settings.regWeight = 0.0;
  settings.ceresOptions.minimizer_progress_to_stdout = false;
  auto Y = filter(observations, settings, Array<Obs1::Vec>());

  EXPECT_EQ(Y.size(), 3);

  EXPECT_NEAR(Y[0](0), 1.2, 1.0e-6);
  EXPECT_NEAR(Y[1](0), 3.4, 1.0e-6);
  EXPECT_NEAR(Y[2](0), 5.6, 1.0e-6);
}

TEST(FilterTest, EffectOfRegularization) {

  Array<Obs1> observations{
    obs(0, 1.0, 1), // 3.4 - 1.1*2 = 3.4 - 2.2 = 1.2
    obs(0, 2.0, 9), // 5.6 - 1.1*2 = 5.6 - 2.2 = 3.4
    obs(0, 4.0, 2)
  };

  {
    Settings settings;
    settings.regWeight = 0.001;
    settings.ceresOptions.minimizer_progress_to_stdout = false;
    auto Y = filter(observations, settings, Array<Obs1::Vec>());
    EXPECT_NEAR(Y[0](0), 1.0, 0.01);
    EXPECT_LT(1.0, Y[0](0));

    EXPECT_NEAR(Y[1](0), 9.0, 0.01);
    EXPECT_LT(Y[1](0), 9.0);

    EXPECT_NEAR(Y[2](0), 2.0, 0.01);
    EXPECT_LT(2.0, Y[2](0));
  }{
    Settings settings;
    settings.regWeight = 1.0e2;
    settings.ceresOptions.minimizer_progress_to_stdout = false;
    auto Y = filter(observations, settings, Array<Obs1::Vec>());

    EXPECT_NEAR(Y[0](0), 4.2857, 0.001);
    EXPECT_NEAR(Y[1](0), 4.0714, 0.001);
    EXPECT_NEAR(Y[2](0), 3.6429, 0.001);
  }
}

TEST(FilterTest, RegularizationAndNonUniformSampling) {
  Array<Obs1> observations{
    obs(0, 1.0, 1),
    obs(0, 1.000001, 2),
    obs(0, 2.0, 9)
  };

  Settings settings;
  settings.regWeight = 1.0e4;
  auto Y = filter(observations, settings, Array<Obs1::Vec>());

  EXPECT_NEAR(0.5*(1 + 2), Y[0](0), 1.0e-4);
  EXPECT_NEAR(0.5*(1 + 2), Y[1](0), 1.0e-4);
  EXPECT_NEAR(9.0, Y[2](0), 1.0e-4);

}
