/*
 * LsqNashSolverTest.cpp
 *
 *  Created on: 12 Jan 2017
 *      Author: jonas
 */
#include <gtest/gtest.h>
#include <server/math/nonlinear/LsqNashSolver.h>

using namespace sail;
using namespace sail::LsqNashSolver;

struct TestFun {
  static const int inputCount = 2;
  static const int outputCount = 2;

  template <typename T>
  void eval(const T *X, T *Y) const {
    T x = X[0];
    T y = X[1];
    Y[0] = sin(x) + y*y;
    Y[1] = 4.0*x*x + cos(3.0*y);
  }
};

Eigen::Matrix2d computeJ(double x, double y) {
  Eigen::Matrix2d Jexpected;
  Jexpected << cos(x), 2*y,
               8*x, -3*sin(3*y);
  return Jexpected;
}

Eigen::Vector2d computeF(double x, double y) {
  return Eigen::Vector2d(sin(x) + y*y, 4*x*x + cos(3*y));
}

TEST(LsqNashSolverTest, PlayerTest) {

  double x = 0.9;
  double y = 4.6;

  Eigen::Vector2d Fexpected = computeF(x, y);

  double expectedValue = Fexpected.squaredNorm();
  Eigen::Matrix2d Jexpected = computeJ(x, y);
  Eigen::Vector2d JtFexpected = Jexpected.transpose()*Fexpected;
  Eigen::Matrix2d JtJexpected = Jexpected.transpose()*Jexpected;

  {
    double X[2] = {x, y};
    double JtFout[2] = {0, 0};
    std::vector<Eigen::Triplet<double>> JtJout;
    ADSubFunction<TestFun> f(Spani(0, 2), {0, 1});
    EXPECT_NEAR(f.eval(X), expectedValue, 1.0e-6);
    EXPECT_NEAR(f.eval(X, JtFout, &JtJout), expectedValue, 1.0e-6);
    EXPECT_NEAR(JtFout[0], JtFexpected(0), 1.0e-6);
    EXPECT_NEAR(JtFout[1], JtFexpected(1), 1.0e-6);
    EXPECT_EQ(JtJout.size(), 4);
    EXPECT_EQ(f.JtJElementCount(), 4);
    Eigen::Matrix2d JtJsum = Eigen::Matrix2d::Zero();
    for (auto x: JtJout) {
      JtJsum(x.row(), x.col()) += x.value();
    }
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        EXPECT_NEAR(JtJexpected(i, j), JtJsum(i, j), 1.0e-6);
      }
    }
  }{
    double X[3] = {0.0, x, y};
    ADSubFunction<TestFun> f(Spani(1, 2), {1, 2});
    double JtFout[3] = {0, 0, 0};
    std::vector<Eigen::Triplet<double>> JtJout;
    EXPECT_NEAR(f.eval(X), expectedValue, 1.0e-6);
    EXPECT_NEAR(f.eval(X, JtFout, &JtJout), expectedValue, 1.0e-6);
    EXPECT_EQ(JtJout.size(), 2);
    EXPECT_EQ(f.JtJElementCount(), 2);
    EXPECT_EQ(JtFout[0], 0.0);
    EXPECT_NEAR(JtFout[1], JtFexpected(0), 1.0e-6);
    EXPECT_EQ(JtFout[2], 0.0);
    Eigen::Matrix<double, 1, 2> JtJsum
      = Eigen::Matrix<double, 1, 2>::Zero();
    for (auto x: JtJout) {
      JtJsum(x.row()-1, x.col()-1) += x.value();
    }
    EXPECT_NEAR(JtJsum(0, 0), JtJexpected(0, 0), 1.0e-6);
    EXPECT_NEAR(JtJsum(0, 1), JtJexpected(0, 1), 1.0e-6);
  }{
    Player p({1, 2});
    double X[3] = {0.0, x, y};
    p.addADSubFunction(TestFun(), {1, 2});
    EXPECT_NEAR(p.eval(X), expectedValue, 1.0e-6);
    EXPECT_EQ(2, p.JtJElementCount());
    EXPECT_EQ(3, p.minInputSize());
    EXPECT_EQ(Spani(1, 2), p.strategySpan());
  }
}


class Square {
public:
  template <typename T>
  static T eval(T x) {
    return x*x;
  }
};

template <typename F>
class BasicFun {
public:
  BasicFun() : _x(0), _y(0) {}
  BasicFun(double x, double y) : _x(x), _y(y) {}

  static const int inputCount = 2;
  static const int outputCount = 2;

  template <typename T>
  void eval(const T *X, T *Y) const {
    T xd = X[0] - _x;
    T yd = X[1] - _y;
    T len2 = 1.0e-30 + xd*xd + yd*yd;
    T len = sqrt(len2);
    T dst = F::template eval<T>(len);
    T f = dst/len2;
    Y[0] = f*xd;
    Y[1] = f*yd;
  }
private:
  double _x, _y;
};

TEST(LsqNashSolverTest, BasicFunTest) {
  auto f = BasicFun<Square>(1, 1);
  {
    double x[2] = {3, 2};
    double y[2] = {0, 0};
    f.eval(x, y);
    EXPECT_NEAR(sqr(y[0]) + sqr(y[1]), 2*2 + 1*1, 1.0e-4);
  }
}
