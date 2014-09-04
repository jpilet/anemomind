/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/Histogram.h>

using namespace sail;

TEST(HistgramTest, BasicMapping) {
  const double marg = 1.0e-6;
  HistogramMap<double, false> h(2, 1.0, 3.0);
  EXPECT_EQ(h.binCount(), 2);
  EXPECT_EQ(h.toBin(1.5), 0);
  EXPECT_EQ(h.toBin(2.5), 1);
  EXPECT_NEAR(h.toLeftBound(0), 1.0, marg);
  EXPECT_NEAR(h.toCenter(0), 1.5, marg);
  EXPECT_NEAR(h.toRightBound(0), 2.0, marg);
  EXPECT_TRUE(h.validIndex(0));
  EXPECT_TRUE(h.validIndex(1));
  EXPECT_FALSE(h.validIndex(2));
}

TEST(HistgramTest, Counting) {
  const double marg = 1.0e-6;
  HistogramMap<double, false> h(2, 1.0, 3.0);
  const int xn = 3;
  double xdata[xn] = {1.2, 1.3, 2.9};
  Arrayd X(xn, xdata);
  Arrayi counts = h.countPerBin(X);
  EXPECT_EQ(counts.size(), 2);
  EXPECT_EQ(counts[0], 2);
  EXPECT_EQ(counts[1], 1);
  Arrayi inds = h.assignBins(X);
  EXPECT_EQ(inds.size(), 3);
  EXPECT_EQ(inds[0], 0);
  EXPECT_EQ(inds[1], 0);
  EXPECT_EQ(inds[2], 1);

  int ydata[xn] = {119, 611, 249};
  Arrayi Y(xn, ydata);
  //Array<Arrayi> groups = h.groupValuesByBin(X, Y);
//  EXPECT_EQ(groups.size(), 2);
//  EXPECT_EQ(groups[0].size(), 2);
//  EXPECT_EQ(groups[1].size(), 1);
//  EXPECT_EQ(groups[1][0], 249);
}


