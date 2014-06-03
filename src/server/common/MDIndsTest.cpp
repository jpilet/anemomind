/*
 *  Created on: 2014-06-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/MDInds.h>
#include <gtest/gtest.h>

TEST(MDIndsTest, Mirror) {
  int sizes[1] = {3};
  sail::MDInds<1> inds(sizes);

  const int n = 6;
  int x[n] = {-1, 0, 1, 2, 3, 4};
  int y[n] = { 0, 0, 1, 2, 2, 1};
  for (int i = 0; i < n; i++) {
    EXPECT_EQ(y[i], inds.calcIndexMirrored(x + i));
  }
}

