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
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

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
  double sum = 0.0;
  EXPECT_TRUE(problem.fillNormalEquations(X, &JtJ, &minusJtF, &sum));
  EXPECT_TRUE(Pbsv<double>::apply(&JtJ, &minusJtF));
  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(minusJtF(i, 0), gtLine(i), 0.1);
  }
  EXPECT_LT(0.0, sum);

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
        MakeConstant<T>::apply(A(0, 0))*x[0] + MakeConstant<T>::apply(A(0, 1))*x[1] + MakeConstant<T>::apply(B(0)),
        MakeConstant<T>::apply(A(1, 0))*x[0] + MakeConstant<T>::apply(A(1, 1))*x[1] + MakeConstant<T>::apply(B(1)));
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

    int counter = -1;

    problem.addIterationCallback([&](const IterationSummary<T> &summary) {
      counter++;
      EXPECT_EQ(summary.iterationsCompleted, counter);
    });
    Eigen::Matrix<T, Eigen::Dynamic, 1> X(1);
    X[0] = T(0.0);
    Settings settings;
    auto results = runLevMar(settings, problem, &X);
    EXPECT_EQ(counter, results.iterationsCompleted);
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

    Eigen::Matrix<ceres::Jet<double, 2>, 2, 1> adTarget(
        jet(1.0, 1.0, 0.0),
        jet(1.0, 0.0, 1.0)
       );

    auto adPt = closestPointOnEllipse<ceres::Jet<double, 2> >(
        A, B, adTarget);

    EXPECT_NEAR(adPt(0).a, k, 1.0e-6);
    EXPECT_NEAR(adPt(1).a, k, 1.0e-6);
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        EXPECT_NEAR(adPt(i).v[j], J(i, j), 1.0e-2);
      }
    }
  }
}


template <typename T>
Angle<T> computeLineAngle(
    const Eigen::Matrix<T, 2, 1> &a,
    const Eigen::Matrix<T, 2, 1> &b) {
  Eigen::Matrix<T, 2, 1> diff = b - a;
  return Angle<T>::radians(atan2(diff(1), diff(0)));
}

Array<Angle<double> > makeAngleData(int n, int inlierCount,
    std::default_random_engine *rng,
    const Angle<double> gtAngle) {
  std::normal_distribution<double> noiseDegrees(0.0, 4.0);
  std::uniform_real_distribution<double> outliersDegrees(0.0, 360.0);
  Array<Angle<double> > dst(n);
  for (int i = 0; i < inlierCount; i++) {
    dst[i] = gtAngle + Angle<double>::degrees(noiseDegrees(*rng));
  }
  for (int i = inlierCount; i < n; i++) {
    dst[i] = Angle<double>::degrees(outliersDegrees(*rng));
  }
  std::shuffle(dst.begin(), dst.end(), *rng);
  return dst;
}

struct PointFit {
  double aWeight, bWeight;
  Eigen::Vector2d dst;
};

Array<PointFit> makePointFitData(
    int n, int inlierCount,
    std::default_random_engine *rng,
    const Eigen::Vector2d &a,
    const Eigen::Vector2d &b) {
  std::normal_distribution<double> noise(0.0, 0.1);
  std::uniform_real_distribution<double> weights(0.0, 1.0);
  std::uniform_real_distribution<double> outliers(-30.0, 30.0);
  Array<PointFit> dst(n);
  for (int i = 0; i < n; i++) {
    double aWeight = weights(*rng);
    double bWeight = 1.0 - aWeight;
    if (i < inlierCount) {
      Eigen::Vector2d at = aWeight*a + bWeight*b;
      dst[i] = PointFit{aWeight, bWeight,
        at + Eigen::Vector2d(noise(*rng), noise(*rng))};
    } else {
      dst[i] = PointFit{aWeight, bWeight,
        Eigen::Vector2d(outliers(*rng), outliers(*rng))};
    }
  }
  std::shuffle(dst.begin(), dst.end(), *rng);
  return dst;
}

template <typename T>
std::pair<Eigen::Matrix<T, 2, 1>, Eigen::Matrix<T, 2, 1> >
  getLineEndpoints(const T *X) {
  auto a = Eigen::Matrix<T, 2, 1>(X[0], X[1]);
  auto b = Eigen::Matrix<T, 2, 1>(X[2], X[3]);
  return std::pair<Eigen::Matrix<T, 2, 1>, Eigen::Matrix<T, 2, 1>>(
      a, b);
}

struct PointFitness {
  double weight;
  PointFit fit;

  static const int inputCount = 4;
  static const int outputCount = 2;

  template <typename T>
  bool evaluate(const T *X, T *y) const {
    auto pts = getLineEndpoints<T>(X);
    Eigen::Matrix<T, 2, 1> err =
        T(weight)*(MakeConstant<T>::apply(fit.aWeight)*pts.first
            + MakeConstant<T>::apply(fit.bWeight)*pts.second
        - fit.dst.cast<T>());
    y[0] = err(0);
    y[1] = err(1);
    return true;
  }
};

struct AngleFitness {
  double weight;
  Angle<double> dst;

  static const int inputCount = 4;
  static const int outputCount = 1;

  template <typename T>
  bool evaluate(const T *X, T *y) const {
    auto pts = getLineEndpoints<T>(X);
    auto angle = computeLineAngle<T>(pts.first, pts.second);
    y[0] = T(weight)*(angle - dst.cast<T>()).normalizedAt0().radians();
    return true;
  }
};

std::pair<Eigen::Vector2d, Eigen::Vector2d>
  solveAngleAndPointFitProblemWithWeights(
      double angleWeight, const Array<Angle<double> > &angles,
      double pointWeight, const Array<PointFit> &points) {
  std::default_random_engine rng;

  Eigen::VectorXd X(4);
  X << 0, 0, 1, 1;


  /*int expectedCount,
        Problem<T> *dstProblem, const LossFunction &loss,
        std::default_random_engine *rng,
        int estSize*/


  Problem<double> problem;
  MeasurementGroup<double, GemanMcClureLoss> angleGroup(
      angles.size(), &problem,
      GemanMcClureLoss(), &rng, angles.size());

  for (auto angle: angles) {
    angleGroup.addCostFunction(Spani(0, 4),
        new AngleFitness{angleWeight, angle});
  }

  MeasurementGroup<double, GemanMcClureLoss> pointGroup(
      points.size(), &problem,
      GemanMcClureLoss(), &rng, points.size());

  for (auto pt: points) {
    pointGroup.addCostFunction(Spani(0, 4),
        new PointFitness{pointWeight, pt});
  }



  return getLineEndpoints<double>(X.data());
}

// Here we demonstrate a technique that lets us
// fit a line using both distances and angle data,
// while rejecting outliers. The only assumption is
// that at least half of the measurements of any kind are
// inliers.
TEST(BandedLevMarTest, MixedAnglesAndDistances) {
  std::default_random_engine rng(0);
  const int dataCount = 30;
  const int commonInlierCount = 18;

  Eigen::Vector2d gtA(2.4, 4.5);
  Eigen::Vector2d gtB(9.6, -7.4);
  auto gtAngle = computeLineAngle(gtA, gtB);

  auto angles = makeAngleData(
      dataCount, commonInlierCount, &rng, gtAngle);

  auto points = makePointFitData(
      dataCount, commonInlierCount, &rng, gtA, gtB);

  // We should end up with the same solution no matter how we
  // weigh the two data series.
  auto ab0 = solveAngleAndPointFitProblemWithWeights(
      1.4, angles, 30.0, points);
  auto ab1 = solveAngleAndPointFitProblemWithWeights(
      30.0, angles, 4.5, points);

  EXPECT_NEAR(ab0.first(0), ab1.first(0), 1.0e-6);
  EXPECT_NEAR(ab0.first(1), ab1.first(1), 1.0e-6);
  EXPECT_NEAR(ab0.second(0), ab1.second(0), 1.0e-6);
  EXPECT_NEAR(ab0.second(1), ab1.second(1), 1.0e-6);
}



