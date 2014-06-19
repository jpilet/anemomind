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

TEST(SpanOverlap, Subset) {
  Spani A(1, 28);
  Spani B(4, 9);
  Array<SpanOverlap> overlaps = computeSpanOverlaps(Array<Spani>::args(A, B));
  EXPECT_EQ(overlaps.size(), 3);


  EXPECT_EQ(overlaps[0], SpanOverlap(Spani(1, 4), Arrayi::args(0)));
  EXPECT_EQ(overlaps[1], SpanOverlap(Spani(4, 9), Arrayi::args(0, 1)));
  EXPECT_EQ(overlaps[2], SpanOverlap(Spani(9, 28), Arrayi::args(0)));
}

TEST(SpanOverlap, Complex) {
  Spani A(1, 28);
  Spani B(4, 9);
  Spani C(4, 12);
  Spani D(5, 11);

  Array<SpanOverlap> overlaps = computeSpanOverlaps(Array<Spani>::args(A, B, C, D));
  EXPECT_EQ(overlaps.size(), 6);
  EXPECT_EQ(overlaps[0], SpanOverlap(Spani(1, 4), Arrayi::args(0)));
  EXPECT_EQ(overlaps[1], SpanOverlap(Spani(4, 5), Arrayi::args(0, 1, 2)));
  EXPECT_EQ(overlaps[2], SpanOverlap(Spani(5, 9), Arrayi::args(0, 1, 2, 3)));
  EXPECT_EQ(overlaps[3], SpanOverlap(Spani(9, 11), Arrayi::args(0, 2, 3)));
  EXPECT_EQ(overlaps[4], SpanOverlap(Spani(11, 12), Arrayi::args(0, 2)));
  EXPECT_EQ(overlaps[5], SpanOverlap(Spani(12, 28), Arrayi::args(0)));
}



