/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/math/NormConstrained.h>
#include <server/math/EigenUtils.h>
#include <gtest/gtest.h>

using namespace sail;
using namespace EigenUtils;
using namespace NormConstrained;

namespace {
  auto rng = makeRngForTests();

  double eval(Eigen::MatrixXd A, Eigen::MatrixXd B, Eigen::MatrixXd X) {
    auto Y = A*X.normalized() - B;
    return Y.norm();
  }
}

TEST(NormConstrainedTest, NormConstrained) {
  int dims = 9;
  auto A = makeRandomMatrix(9, dims, &rng);
  auto B = makeRandomMatrix(9, 1, &rng);
  auto Xinit = makeRandomMatrix(dims, 1, &rng);

  Settings s;
  auto X = minimizeNormConstrained(A, B, Xinit, s);
  auto ref = eval(A, B, X);
  for (int i = 0; i < 30; i++) {
    auto X0 = X + makeRandomMatrix(4, 1, &rng, 1.0e-3);
    EXPECT_LE(ref, eval(A, B, X0));
  }
}
