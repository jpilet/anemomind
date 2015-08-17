/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/nnls.h>

using namespace sail;

TEST(nnlsTest, TestClosestPoint) {
  MDArray2d A(3, 2);
  A.setAll(0.0);
  A(0, 0) = 1.0;
  A(1, 1) = 1.0;

  {
    auto x = NNLS::solve(A, Arrayd{-1, -1, 123});
    EXPECT_TRUE(x.successful());
    EXPECT_NEAR(x.X()[0], 0.0, 1.0e-6);
    EXPECT_NEAR(x.X()[1], 0.0, 1.0e-6);
  }{
    auto x = NNLS::solve(A, Arrayd{-9, 13, 9964});
    EXPECT_TRUE(x.successful());
    EXPECT_NEAR(x.X()[0], 0.0, 1.0e-6);
    EXPECT_NEAR(x.X()[1], 13.0, 1.0e-6);
  }{
    auto x = NNLS::solve(A, Arrayd{3, 89, 12222});
    EXPECT_TRUE(x.successful());
    EXPECT_NEAR(x.X()[0], 3.0, 1.0e-6);
    EXPECT_NEAR(x.X()[1], 89.0, 1.0e-6);
  }
}
