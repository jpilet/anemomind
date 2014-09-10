/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/hmm/QuantFilter.h>
#include <cmath>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>

using namespace sail;

TEST(QuantFilterTest, StepTest) {
  LineKM map(-2, 2, -1, 1);

  const int count = 300;
  MDArray2d data(1, count);
  for (int i = 0; i < count; i++) {
    data(0, i) = (2*i < count? -1 : 1) + 2.0*(sin(234234.234*i)*0.34 + sin(98734.234324*i)*0.3898);
  }

  double lambda = 10; //4;
  MDArray2i inds0 = quantFilter(Array<LineKM>::args(map), data, 0);
  MDArray2i inds1 = quantFilter(Array<LineKM>::args(map), data, lambda);

  int withoutReg[count] = {-2, -2, -1, -2, 0, -2, 1, -2, 1, -2, 0, -2, -1, -3, -3, -3, -4, -3, -5, -2, -5, -2, -4, -1, -3, -1, -2, -1, -1, -1, 0, -1, 1, -2, 0, -3, -1, -3, -2, -4, -3, -4, -4, -3, -4, -2, -4, -1, -3, 0, -3, 0, -2, 0, -1, 0, 0, -1, 0, -2, -1, -4, -1, -4, -2, -5, -3, -4, -3, -3, -3, -2, -3, -1, -3, 0, -2, 1, -2, 1, -1, 0, -1, -1, -1, -3, -2, -4, -2, -5, -2, -5, -2, -4, -2, -3, -2, -2, -2, 0, -2, 1, -2, 1, -2, 1, -2, 0, -3, -2, -3, -3, -3, -4, -3, -5, -2, -5, -2, -4, -1, -3, -1, -2, -1, 0, -1, 0, -2, 0, -2, 0, -3, -1, -4, -2, -4, -3, -4, -4, -3, -4, -2, -4, -1, -3, 0, -2, 0, -1, 4, 3, 3, 4, 2, 4, 1, 3, 0, 3, 0, 2, -1, 1, 0, 1, 1, 1, 2, 1, 4, 2, 4, 2, 5, 2, 5, 3, 4, 3, 2, 2, 1, 2, 0, 2, -1, 2, -1, 2, 0, 2, 1, 2, 3, 2, 4, 2, 5, 2, 5, 2, 4, 2, 3, 1, 2, 1, 1, 1, 0, 1, -1, 2, -1, 3, 0, 3, 1, 3, 3, 3, 4, 3, 4, 2, 4, 2, 4, 1, 3, 0, 2, 0, 1, 0, 0, 1, 0, 2, 0, 3, 1, 4, 2, 4, 3, 4, 3, 3, 4, 2, 3, 1, 3, 0, 2, -1, 2, -1, 1, 0, 1, 1, 1, 3, 1, 4, 2, 5, 2, 5, 2, 4, 3, 3, 2, 2, 2, 1, 2, 0, 2, -1, 2, -1, 2, 0, 2, 1, 2, 3, 2, 4, 2, 5, 2, 5, 2, 4};
  int withReg[count] = {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -1, -1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

  for (int i = 0; i < count; i++) {
    EXPECT_EQ(withoutReg[i], inds0(0, i));
    EXPECT_EQ(withReg[i], inds1(0, i));
  }
}
