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
  for (int i = 0; i < 2; i++) {
    EXPECT_NEAR(coefs[i], gtCoefs[i], 1.0e-3);
  }

  double gtValue = 0.9209;
  EXPECT_NEAR(x.eval(coefs), gtValue, 1.0e-3);
}



