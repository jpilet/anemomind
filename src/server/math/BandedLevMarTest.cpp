/*
 * BandedLevMarTest.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#include <server/common/ArrayIO.h>
#include <server/math/BandedLevMar.h>
#include <gtest/gtest.h>
#include <random>
#include <server/common/LineKM.h>

using namespace sail;
using namespace sail::BandedLevMar;

namespace {
  struct DataCost{
    double data;

    static const int inputCount = 1;
    static const int outputCount = 1;

    template <typename T>
    bool evaluate(const T *X, T *y) const {
      y[0] = X[0] - data;
      return true;
    }
  };

  struct RegCost {
    double weight;
    static const int inputCount = 3;
    static const int outputCount = 1;

    template <typename T>
    bool evaluate(const T *X, T *y) const {
      y[0] = weight*(X[0] - 2.0*X[1] + X[2]);
      return true;
    }
  };

}

TEST(BandedLevmarTest, BasicLineFit) {
  std::default_random_engine rng(0);

  std::uniform_real_distribution<double> distrib(0.0, 0.1);

  const int n = 30;

  LineKM gtLine(0, n, 3.4, 9.5);

  Problem<double> problem;
  for (int i = 0; i < n; i++) {
    problem.addCostFunction(Spani(i, i+1),
        new DataCost{gtLine(i) + distrib(rng)});
  }

  EXPECT_EQ(0, problem.kd());
  EXPECT_EQ(n, problem.paramCount());
  EXPECT_EQ(n, problem.residualCount());

  double reg = 1000;

  for (int i = 1; i < n-1; i++) {
    problem.addCostFunction(Spani(i-1, i+2), new RegCost{reg});
  }
  EXPECT_EQ(2, problem.kd());
  EXPECT_EQ(n, problem.paramCount());
  EXPECT_EQ(n + n-2, problem.residualCount());

  double X[n];
  for (int i = 0; i < n; i++) {
    X[i] = 0.0;
  }

  SymmetricBandMatrixL<double> JtJ;
  MDArray2d minusJtF;
  EXPECT_TRUE(problem.fillNormalEquations(X, &JtJ, &minusJtF));
  EXPECT_TRUE(Pbsv<double>::apply(&JtJ, &minusJtF));
  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(minusJtF(i, 0), gtLine(i), 0.1);
  }

  Eigen::VectorXd Xe = Eigen::VectorXd::Zero(n);

  Settings settings;
  auto results = runLevmar(settings, problem, &Xe);
  EXPECT_TRUE(results.success());
  EXPECT_EQ(Xe.size(), n);
  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(Xe(i), minusJtF(i, 0), 1.0e-6);
  }
}



