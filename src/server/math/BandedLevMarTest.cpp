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

  double reg = 1.0; //1000;

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
  settings.verbosity = 1;
  auto results = runLevMar(settings, problem, &Xe);
  EXPECT_TRUE(results.success());
  EXPECT_EQ(Xe.size(), n);
  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(Xe(i), minusJtF(i, 0), 1.0e-6);
  }
}

namespace {

  template <typename T>
  Eigen::Matrix<T, 2, 1> mapAngleToEllipse(
      const Eigen::Matrix2d &A,
      const Eigen::Vector2d &B,
      T angle) {
    T x[2] = {cos(angle), sin(angle)};
    return Eigen::Matrix<T, 2, 1>(
        T(A(0, 0))*x[0] + T(A(0, 1))*x[1] + T(B(0)),
        T(A(1, 0))*x[0] + T(A(1, 1))*x[1] + T(B(1)));
  }

  template <typename T>
  struct EllipseFittingCost {
    static const int inputCount = 1;
    static const int outputCount = 2;
    Eigen::Matrix2d A;
    Eigen::Vector2d B;
    Eigen::Matrix<T, 2, 1> target;

    template <typename S>
    bool evaluate(const S *angle, S *y) const {
      auto x = mapAngleToEllipse(A, B, angle[0]);
      y[0] = x[0] - S(target(0));
      y[1] = x[1] - S(target(1));
      return true;
    }
  };

/*
 * Returns the closest point to 'target'
 * on the ellipse, defined by an affine transformation
 * applied to a unit circle.
 */
  template <typename T>
  Eigen::Matrix<T, 2, 1> closestPointOnEllipse(
      const Eigen::Matrix2d &A,
      const Eigen::Vector2d &B,
      Eigen::Matrix<T, 2, 1> &target) {
    Problem<T> problem;
    problem.addCostFunction(Spani(0, 1),
        new EllipseFittingCost<T>{A, B, target});
    Eigen::Matrix<T, Eigen::Dynamic, 1> X(1);
    X[0] = T(0.0);
    Settings settings;
    runLevMar(settings, problem, &X);
    return mapAngleToEllipse(A, B, X(0));
  }

  Eigen::Matrix2d closestPointDerivative(
      const Eigen::Matrix2d &A,
      const Eigen::Vector2d &B,
      const Eigen::Vector2d &X) {
    Eigen::Matrix2d J(2, 2);
    double h = 1.0e-5;
    double f = 1.0/(2.0*h);
    for (int i = 0; i < 2; i++) {
      Eigen::Vector2d xTmp = X;
      xTmp(i) += h;
      Eigen::Vector2d fPlus = closestPointOnEllipse<double>(A, B, xTmp);
      xTmp = X;
      xTmp(i) -= h;
      Eigen::Vector2d fMinus = closestPointOnEllipse<double>(A, B, xTmp);
      J.block(0, i, 2, 1) = f*(fPlus - fMinus);
    }
    return J;
  }

  ceres::Jet<double, 2> jet(double a, double dx, double dy) {
    ceres::Jet<double, 2> dst(a);
    dst.v[0] = dx;
    dst.v[1] = dy;
    return dst;
  }
}

TEST(BandedLevMarTest, Differentiable) {
  {
    Eigen::Matrix2d A = Eigen::Matrix2d::Identity();
    Eigen::Vector2d B = Eigen::Vector2d::Zero();
    Eigen::Vector2d target(1, 1);
    auto pt = closestPointOnEllipse(A, B, target);
    double k = 1.0/sqrt(2.0);
    EXPECT_NEAR(pt(0), k, 1.0e-6);
    EXPECT_NEAR(pt(1), k, 1.0e-6);

    Eigen::Matrix2d J = closestPointDerivative(A, B, target);
    std::cout << " J numeric = \n" << J << std::endl;

    Eigen::Matrix<ceres::Jet<double, 2>, 2, 1> adTarget(
        jet(1.0, 1.0, 0.0),
        jet(1.0, 0.0, 1.0)
       );

    auto adPt = closestPointOnEllipse<ceres::Jet<double, 2> >(
        A, B, adTarget);

  }
}



