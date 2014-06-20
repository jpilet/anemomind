/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/SpanOverlap.h>
#include <server/common/ArrayIO.h>

using namespace sail;

TEST(SpanOverlap, EmptyTest) {
  EXPECT_TRUE(SpanOverlapi::compute(Array<Spani>(),
      Arrayi()).empty());
}

TEST(SpanOverlap, Partial) {
  Spani A(1, 9);
  Spani B(4, 28);
  Array<SpanOverlapi> overlaps = SpanOverlapi::compute(Array<Spani>::args(A, B), Arrayi::args(0, 1));
  EXPECT_EQ(overlaps.size(), 3);


  EXPECT_EQ(overlaps[0], SpanOverlapi(Spani(1, 4), Arrayi::args(0)));
  EXPECT_EQ(overlaps[1], SpanOverlapi(Spani(4, 9), Arrayi::args(0, 1)));
  EXPECT_EQ(overlaps[2], SpanOverlapi(Spani(9, 28), Arrayi::args(1)));
}

TEST(SpanOverlap, Subset) {
  Spani A(1, 28);
  Spani B(4, 9);
  Array<SpanOverlapi> overlaps = SpanOverlapi::compute(Array<Spani>::args(A, B), Arrayi::args(0, 1));
  EXPECT_EQ(overlaps.size(), 3);


  EXPECT_EQ(overlaps[0], SpanOverlapi(Spani(1, 4), Arrayi::args(0)));
  EXPECT_EQ(overlaps[1], SpanOverlapi(Spani(4, 9), Arrayi::args(0, 1)));
  EXPECT_EQ(overlaps[2], SpanOverlapi(Spani(9, 28), Arrayi::args(0)));
}

TEST(SpanOverlap, Complex) {
  Spani A(1, 28);
  Spani B(4, 9);
  Spani C(4, 12);
  Spani D(5, 11);

  Array<SpanOverlapi> overlaps = SpanOverlapi::compute(Array<Spani>::args(A, B, C, D), Arrayi::args(0, 1, 2, 3));
  EXPECT_EQ(overlaps.size(), 6);
  EXPECT_EQ(overlaps[0], SpanOverlapi(Spani(1, 4), Arrayi::args(0)));
  EXPECT_EQ(overlaps[1], SpanOverlapi(Spani(4, 5), Arrayi::args(0, 1, 2)));
  EXPECT_EQ(overlaps[2], SpanOverlapi(Spani(5, 9), Arrayi::args(0, 1, 2, 3)));
  EXPECT_EQ(overlaps[3], SpanOverlapi(Spani(9, 11), Arrayi::args(0, 2, 3)));
  EXPECT_EQ(overlaps[4], SpanOverlapi(Spani(11, 12), Arrayi::args(0, 2)));
  EXPECT_EQ(overlaps[5], SpanOverlapi(Spani(12, 28), Arrayi::args(0)));
}

TEST(SpanOverlap, SubsetObjs) {
  Spani A(1, 28);
  Spani B(4, 9);
  Array<char> objs = Array<char>::args('A', 'B');


  Array<SpanOverlap<char> > overlaps =
      SpanOverlap<char>::compute(Array<Spani>::args(A, B), objs);

  EXPECT_EQ(overlaps.size(), 3);
  EXPECT_EQ(overlaps[0].objects(), Array<char>::args('A'));
  EXPECT_EQ(overlaps[1].objects(), Array<char>::args('A', 'B'));
  EXPECT_EQ(overlaps[2].objects(), Array<char>::args('A'));
}
