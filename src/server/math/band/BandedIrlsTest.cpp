/*
 * BandedIrlsTest.cpp
 *
 *  Created on: 30 Mar 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/math/band/BandedIrls.h>
#include <server/math/band/BandedIrlsUtils.h>
#include <server/common/ArrayBuilder.h>

using namespace sail;
using namespace sail::BandedIrls;

namespace {

class BasicCost : public Cost {
public:
  BasicCost(
      int offset,
      const Eigen::Matrix2d& A,
      const Eigen::Vector2d& B) : _offset(offset), _A(A), _B(B) {}

  void initialize(BandProblem* dst) override {
    dst->addNormalEquations<2, 2, 1>(
        _offset,
        Eigen::Matrix2d::Identity(),
        Eigen::Vector2d::Zero());
  }

  void constantApply(
        BandProblem* dst) const override {
    dst->addNormalEquations<2, 2, 1>(_offset, _A, _B);
  }

    int minimumProblemDimension() const override {
      return _offset + 2;
    }

    int maximumDiagonalWidth() const override {
      return 2;
    }
private:
  int _offset;
  Eigen::Matrix2d _A;
  Eigen::Vector2d _B;
};

}

TEST(BandedIrlsTest, BasicSolving) {
  Eigen::Matrix2d A0;
  A0 << 0, 2,
        4, 0;
  Eigen::Vector2d B0(9, 14);

  Eigen::Matrix2d A1;
  A1 << 5, 0,
        0, 4;
  Eigen::Vector2d B1(15, 20);

  Settings options;
  auto solution = solve(options, {
      Cost::Ptr(new BasicCost(0, A0, B0)),
      Cost::Ptr(new BasicCost(2, A1, B1))
  });
  auto X = solution.X;
  EXPECT_NEAR(X(0, 0), 3.5, 1.0e-5);
  EXPECT_NEAR(X(1, 0), 4.5, 1.0e-5);
  EXPECT_NEAR(X(2, 0), 3.0, 1.0e-5);
  EXPECT_NEAR(X(3, 0), 5.0, 1.0e-5);
}

Eigen::Matrix<double, 1, 1> mat1x1(double x) {
  Eigen::Matrix<double, 1, 1> m;
  m(0, 0) = x;
  return m;
}

TEST(BandedIrlsTest, LineFit) {
  Settings options;
  options.iterations = 30;
  ExponentialWeighting weights(options.iterations, 0.1, 10000.0);

  int samples = 8;

  Eigen::Matrix<double, 1, 3> coefs;
  coefs << 1, -2, 1;

  ArrayBuilder<Cost::Ptr> costs(2*samples);
  for (int i = 0; i < samples-2; i++) {
    costs.add(Cost::Ptr(new StaticCost<1,3,1>(i, coefs, mat1x1(0.0))));
  }

  LineKM gt(0, samples-1, 3, 9);
  for (int i = 0; i < samples; i++) {
    auto y = i == 4? 100 : gt(i) + 0.1*sin(30*sin(34.4*i));
    costs.add(Cost::Ptr(new RobustCost<1, 1, 1>(i, mat1x1(1.0), mat1x1(y),
        weights, 1.0)));
  }

  auto solution = solve(options, costs.get());

  for (int i = 0; i < samples; i++) {
    EXPECT_NEAR(solution.X(i, 0), gt(i), 0.2);
  }
}
