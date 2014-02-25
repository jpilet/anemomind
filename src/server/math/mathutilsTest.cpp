/*
 *  Created on: 2014-02-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "mathutils.h"
#include <gtest/gtest.h>

using namespace sail;

TEST(mathutilsTest, kronWithSpEyeTest) {
  double vals[3] = {-34.3, 0.1, 119.0};
  for (int R = 0; R < 3; R++) {
    double val = vals[R];
    arma::umat IJ(2, 1);
    IJ(0, 0) = 0;
    IJ(1, 0) = 0;
    arma::vec X(1);
    X[0] = val;
    arma::sp_mat K(IJ, X, 2, 2);
    arma::sp_mat K2 = kronWithSpEye(K, 2);
    EXPECT_EQ(K2.n_cols, 4);
    EXPECT_EQ(K2.n_rows, 4);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        double x = K2(i, j);
        if (i == j && i < 2) {
          EXPECT_NEAR(x, val, 1.0e-9);
        } else {
          EXPECT_NEAR(x, 0.0, 1.0e-9);
        }
      }
    }
  }
}


