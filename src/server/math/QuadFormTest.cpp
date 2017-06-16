/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/QuadForm.h>
#include <armadillo>
#include <gtest/gtest.h>

#include <server/common/LineKM.h>

using namespace sail;

TEST(QuadFormTest, LineFitTest) {
  double x[2] = {1.0, 3.9};
  double y[2] = {2.34, 4.0013};
  QuadForm<2, 1> q = LineFitQF::fitLine(x[0], y[0])
                   + LineFitQF::fitLine(x[1], y[1]);
  double coefs[2];
  q.minimize2x1(coefs);

  MDArray2d coefs2 = q.minimize();
  EXPECT_EQ(coefs2.rows(), 2);
  EXPECT_EQ(coefs2.cols(), 1);
  EXPECT_NEAR(coefs[0], coefs2(0, 0), 1.0e-6);
  EXPECT_NEAR(coefs[1], coefs2(1, 0), 1.0e-6);

  LineKM truth(x[0], x[1], y[0], y[1]);
  EXPECT_NEAR(coefs[0], truth.getK(), 1.0e-6);
  EXPECT_NEAR(coefs[1], truth.getM(), 1.0e-6);
}

TEST(QuadFormTest, LineFitWithCost) {
  const int count = 3;
  double X[count] = {-0.8045, 0.6966, 0.8351};
  double Y[count] = { -0.2437, 0.2157, -1.1658};
  const double gtCoefs[2] = {-0.2043, -0.3484};

  LineFitQF x = 0;
  for (int i = 0; i < count; i++) {
    x += LineFitQF::fitLine(X[i], Y[i]);
  }

  double coefs[2];
  x.minimize2x1(coefs);

  auto coefsEigen = x.minimizeUsingEigen();

  for (int i = 0; i < 2; i++) {
    EXPECT_NEAR(coefs[i], gtCoefs[i], 1.0e-3);
    EXPECT_NEAR(gtCoefs[i], coefsEigen(i), 1.0e-3);
  }

  double gtValue = 0.9209;
  EXPECT_NEAR(x.eval(coefs), gtValue, 1.0e-3);

  auto ecost = x.evaluateEigenCost(coefsEigen);
  EXPECT_NEAR(ecost, gtValue, 1.0e-3);
}

TEST(QuadFormTest, SliceTest) {
  double X[3] = {1, 2, 3};
  double Y[2] = {9, 7};
  auto qf = QuadForm<3, 2, double>::fit(X, Y);
  auto qf2 = qf.zeroPaddedSlice<2>(1);
  EXPECT_EQ(qf2.pElement(0, 0), qf.pElement(1, 1));
  EXPECT_EQ(qf2.pElement(1, 0), qf.pElement(2, 1));
  EXPECT_EQ(qf2.pElement(0, 1), qf.pElement(1, 2));

  EXPECT_EQ(qf2.qElement(0, 1), qf.qElement(1, 1));
  EXPECT_EQ(qf2.rElement(0, 0), qf.rElement(0, 0));

  auto qf3 = qf.zeroPaddedSlice<2>(-1);
  EXPECT_EQ(qf3.pElement(0, 0), 0.0);
  EXPECT_EQ(qf3.pElement(1, 1), qf.pElement(0, 0));
}

TEST(QuadFormTest, SplitInds) {
  std::array<bool, 5> mask{true, true, false, false, true};
  IndexSplit<5> split(mask);
  EXPECT_EQ(split.trueInds(), (Array<int>{0, 1, 4}));
  EXPECT_EQ(split.falseInds(), (Array<int>{2, 3}));
  EXPECT_EQ(split.allInds(), (Array<int>{0, 1, 2, 3, 4}));

  int counter = 0;
  Eigen::MatrixXd fullMatrix(5, 5);
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      fullMatrix(i, j) = counter;
      counter++;
    }
  }
  Eigen::MatrixXd sub = subMatrixByInds(
      fullMatrix, split.trueInds(), split.falseInds());
  EXPECT_EQ(sub.rows(), 3);
  EXPECT_EQ(sub.cols(), 2);
  EXPECT_EQ(sub(0, 0), 2.0);
  EXPECT_EQ(sub(2, 1), 23.0);
}

TEST(QuadFormTest, Elimination) {
  typedef QuadForm<3, 1> QF;
  QF qf = QF::fit(
      std::array<double, 3>{1, -2, 1}.data(),
      std::array<double, 1>{0}.data());
  auto e = qf.eliminate(std::array<bool, 3>{true, false, true});
  Eigen::MatrixXd P = e.eigenP();
  Eigen::MatrixXd Q = e.eigenQ();
  Eigen::MatrixXd R = e.eigenR();
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_NEAR(P(i, j), 0.0, 1.0e-9);
    }
    EXPECT_NEAR(Q(i, 0), 0.0, 1.0e-9);
  }
  EXPECT_NEAR(R(0, 0), 0.0, 1.0e-9);
}

TEST(QuadFormTest, MinimalExampleElimination) {
  typedef std::array<double, 3> A2;
  typedef std::array<double, 1> A1;
  typedef QuadForm<2, 1> QF;

  auto qf = QF::fit(A2{1, 1}.data(), A1{1}.data());

  EXPECT_NEAR(qf.evaluateOptimalEigenCost(), 0.0, 1.0e-6);

  auto qfe2 = qf.eliminate(std::array<bool, 2>{false, true});
  auto qfe = qfe2.zeroPaddedSlice<1>(0);

  EXPECT_NEAR(qfe.evaluateOptimalEigenCost(), 0.0, 1.0e-6);
}

TEST(QuadFormTest, Elimination2) {
  typedef std::array<double, 3> A3;
  typedef std::array<double, 1> A1;
  typedef QuadForm<3, 1> QF;
  QF qf = QF::fit(A3{9, 3, 4}.data(), A1{93}.data())
    + QF::fit(A3{9, 5, 30}.data(), A1{7}.data())
    + QF::fit(A3{0, 2, -4}.data(), A1{9}.data())
    + QF::fit(A3{1, 24, 1}.data(), A1{-3}.data());


  auto costA = qf.evaluateOptimalEigenCost();
  auto qfe3 = qf.eliminate(std::array<bool, 3>{true, false, true});
  auto qfe = qfe3.zeroPaddedSlice<2>(0);
  auto costB = qfe.evaluateOptimalEigenCost();

  EXPECT_NEAR(costA, costB, 1.0e-6);
}
