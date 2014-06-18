/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/SpanOverlap.h>
#include <server/common/ArrayIO.h>

using namespace sail;

TEST(SpanOverlap, EmptyTest) {
  EXPECT_TRUE(computeSpanOverlaps(Array<Spani>()).empty());
}

TEST(SpanOverlap, Partial) {
  Spani A(1, 9);
  Spani B(4, 28);
  Array<SpanOverlap> overlaps = computeSpanOverlaps(Array<Spani>::args(A, B));
  EXPECT_EQ(overlaps.size(), 3);


  EXPECT_EQ(overlaps[0], SpanOverlap(Spani(1, 4), Arrayi::args(0)));
  EXPECT_EQ(overlaps[1], SpanOverlap(Spani(4, 9), Arrayi::args(0, 1)));
  EXPECT_EQ(overlaps[2], SpanOverlap(Spani(9, 28), Arrayi::args(1)));
}


