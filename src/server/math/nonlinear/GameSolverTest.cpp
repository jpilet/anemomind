/*
 * GameSolverTest.cpp
 *
 *  Created on: 29 Dec 2016
 *      Author: jonas
 */

#include <server/math/nonlinear/GameSolver.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {

  template <typename T>
  T v(const Array<T> &x) {
    return x[0];
  }

  adouble playerA(const Array<Array<adouble>> &X) {
    adouble a = v(X[0])*9.3 + v(X[1])*4.7 + 3;
    adouble b = -v(X[0])*2.9 + 7.5*v(X[1]) - 8;
    return a*a + b*b;
  }

  adouble playerB(const Array<Array<adouble>> &X) {
    adouble a = v(X[0])*2 + v(X[1])*3 - 4;
    adouble b = v(X[0])*5 + 6*v(X[1]) - 7;
    return a*a + b*b;
  }

  // Returns the derivative of playerA objective w.r.t.
  // the first parameter block in X, and the derivative
  // of playerB w.r.t. the second parameter block in X.
  // At the Nash equilibrium, those two values should
  // be close to 0, so we can check that in our unit
  // tests.
  std::pair<double, double> computePartialDerivatives(
      const Array<Array<double>> &X,
      const GameSolver::Settings &settings) {
    double dAd1 = 1.0e6;
    double dBd2 = 1.0e6;
    GameSolver::evaluatePartialGradient(
          0, &playerA,
          X, settings, &dAd1);
    GameSolver::evaluatePartialGradient(
          1, &playerB,
          X, settings, &dBd2);
    return std::pair<double, double>(dAd1, dBd2);
  }

  void disp(int iter, const Array<Array<double>> &X) {
    std::cout << "## Iteration " << iter << std::endl;
    std::cout << "  Estimate:    " << v(X[0]) << ", " << v(X[1]) << std::endl;
    auto p = computePartialDerivatives(X, GameSolver::Settings());
    std::cout << "  Derivatives: " << p.first << ", " << p.second << std::endl;
  }
}

TEST(GameSolverTest, TwoPlayers) {
  GameSolver::Settings settings;

  // This parameter needs to be tuned.
  // The smaller, the more stable convergence but slower.
  // For a value which is too high it might not converge.
  //settings.stepSize = 0.001;

  //settings.stepManagerPrototype = GameSolver::StepManager::Ptr(
  //    new GameSolver::ConstantStepManager(0.001));

  sail::RNG rng;

  GameSolver::RandomStepManager::Settings rs;
  rs.verbose = true;
  rs.rng = &rng;
  rs.logInitialStepMu = log(100.0);
  rs.subSampleSize = 5;

  settings.stepManagerPrototype = GameSolver::StepManager::Ptr(
      new GameSolver::RandomStepManager(rs));
  settings.verbose = true;

  settings.iterationCallback = &disp;

  Array<Array<double>> Xinit{{0}, {0}};

  auto derInit = computePartialDerivatives(Xinit, settings);
  EXPECT_LT(10, std::abs(derInit.first));
  EXPECT_LT(10, std::abs(derInit.second));

  auto Xopt = GameSolver::optimize(
      {&playerA, &playerB}, Xinit, settings);

  auto derOpt = computePartialDerivatives(Xopt, settings);
  EXPECT_LT(std::abs(derOpt.first), 0.1);
  EXPECT_LT(std::abs(derOpt.second), 0.1);
}



