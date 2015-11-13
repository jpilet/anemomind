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
}

TEST(NormConstrainedTest, NormConstrained) {
  auto A = makeRandomMatrix(9, 4, &rng);
  auto B = makeRandomMatrix(9, 1, &rng);
  auto Xinit = makeRandomMatrix(3, 1, &rng);
  Settings s;
  auto X = minimizeNormConstrained(A, B, Xinit, s);

}
