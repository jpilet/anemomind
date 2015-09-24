/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/nonlinear/SparseCurveFit.h>

using namespace sail;
using namespace SparseCurveFit;


bool operator== (Triplet a, Triplet b) {
  return a.row() == b.row() && a.col() == b.col() && a.value() == b.value();
}

TEST(SparseCurveFitTest, RegTest) {
  std::vector<Triplet> dst;
  auto spans = makeReg(2, 10, 1,
      2, 2, &dst);
  EXPECT_EQ(spans.size(), 2);
  EXPECT_EQ(spans[0], Spani(10, 12));
  EXPECT_EQ(spans[1], Spani(12, 14));

  EXPECT_TRUE(dst[0] == Triplet(10, 1, 1.0));
  EXPECT_TRUE(dst[1] == Triplet(11, 2, 1.0));
  EXPECT_TRUE(dst[2] == Triplet(10, 3, -2.0));
  EXPECT_TRUE(dst[3] == Triplet(11, 4, -2.0));
  EXPECT_TRUE(dst[4] == Triplet(10, 5, 1.0));
  EXPECT_TRUE(dst[5] == Triplet(11, 6, 1.0));

  EXPECT_TRUE(dst[6] == Triplet(12, 3, 1.0));
  EXPECT_TRUE(dst[7] == Triplet(13, 4, 1.0));
  EXPECT_TRUE(dst[8] == Triplet(12, 5, -2.0));
  EXPECT_TRUE(dst[9] == Triplet(13, 6, -2.0));
  EXPECT_TRUE(dst[10] == Triplet(12, 7, 1.0));
  EXPECT_TRUE(dst[11] == Triplet(13, 8, 1.0));

}
