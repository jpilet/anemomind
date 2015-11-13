/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/math/EigenUtils.h>

using namespace sail;
using namespace EigenUtils;

namespace {
  auto rng = makeRngForTests();

  double evalNormRatio(Eigen::MatrixXd A, Eigen::MatrixXd B, Eigen::VectorXd X) {
    auto a = A*X;
    auto b = B*X;
    return a.squaredNorm()/b.squaredNorm();
  }

  double evalNormRatio(Eigen::MatrixXd A, Eigen::VectorXd B,
                          Eigen::MatrixXd C, Eigen::VectorXd D, Eigen::VectorXd X) {
    auto a = A*X + B;
    auto b = C*X + D;
    return a.squaredNorm()/b.squaredNorm();
  }
}

TEST(EigenUtilsTest, Compress) {
  auto A = makeRandomMatrix(9, 3, &rng);
  Eigen::VectorXd B = makeRandomMatrix(9, 1, &rng);
  auto c = compress(ABPair{A, B});
  Eigen::MatrixXd AtA = A.transpose()*A;
  Eigen::MatrixXd AtB = A.transpose()*B;
  Eigen::MatrixXd AtA2 = c.A.transpose()*c.A;
  Eigen::MatrixXd AtB2 = c.A.transpose()*c.B;
  EXPECT_TRUE(eq(AtA, AtA2));
  EXPECT_TRUE(eq(AtB, AtB2));
}

TEST(EigenUtilsTest, OrthoBasis) {
  auto A = makeRandomMatrix(9, 3, &rng);
  auto B = makeRandomMatrix(9, 3, &rng);
  auto Q = orthonormalBasis(A);
  EXPECT_FALSE(isOrthonormal(A));
  EXPECT_TRUE(isOrthonormal(Q));
  EXPECT_TRUE(spanTheSameSubspace(A, Q));
  EXPECT_FALSE(spanTheSameSubspace(A, B));
}

TEST(EigenUtilsTest, MinimizeNormFraction) {
  auto A = makeRandomMatrix(9, 3, &rng, 1.0);
  auto B = makeRandomMatrix(4, 3, &rng, 1.0);
  auto X = minimizeNormRatio(A, B);
  EXPECT_EQ(X.rows(), 3);
  EXPECT_EQ(X.cols(), 1);
  auto val = evalNormRatio(A, B, X);
  for (int i = 0; i < 12; i++) {
    EXPECT_LE(val, evalNormRatio(A, B, X + makeRandomMatrix(3, 1, &rng, 0.01)));
  }
}

TEST(EigenUtilsTest, Nullspace) {
  Eigen::MatrixXd K = Eigen::MatrixXd::Identity(3, 2);
  auto L = nullspace(K);
  EXPECT_EQ(L.rows(), 3);
  EXPECT_EQ(L.cols(), 1);
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(L(i, 0), (i == 2? 1.0: 0.0), 1.0e-6);
  }
}

TEST(EigenUtilsTest, Nullspace2) {
  auto k = makeRandomMatrix(3, 1, &rng);
  auto A = nullspace(k);
  EXPECT_EQ(A.rows(), 3);
  EXPECT_EQ(A.cols(), 2);
  Eigen::MatrixXd Atk = A.transpose()*k;
  EXPECT_TRUE(eq(Atk, Eigen::MatrixXd::Zero(2, 1)));
  EXPECT_FALSE(eq(A, Eigen::MatrixXd::Zero(3, 2)));
}


TEST(EigenUtilsTest, MinimizeNormFraction2) {
  auto A = makeRandomMatrix(9, 3, &rng, 1.0);
  auto B = makeRandomMatrix(9, 1, &rng, 1.0);
  auto C = makeRandomMatrix(5, 3, &rng, 1.0);
  auto D = makeRandomMatrix(5, 1, &rng, 1.0);
  auto X = minimizeNormRatio(A, B, C, D);
  EXPECT_EQ(X.rows(), 3);
  EXPECT_EQ(X.cols(), 1);
  auto val = evalNormRatio(A, B, C, D, X);
  for (int i = 0; i < 12; i++) {
    EXPECT_LE(val, evalNormRatio(A, B, C, D, X + makeRandomMatrix(3, 1, &rng, 0.01)));
  }
}
