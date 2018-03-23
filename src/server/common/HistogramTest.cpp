/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/Histogram.h>
#include <server/common/LineKM.h>

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
}







TEST(HistogramTest, Undefined) {
  HistogramMap<Angle<double>, true> map;
  EXPECT_FALSE(map.defined());
  EXPECT_TRUE(map.undefined());
}

 TEST(HistogramTest, BasicTest) {
  HistogramMap<Angle<double>, true> hmap = makePolarHistogramMap<double>(3);
  EXPECT_FALSE(hmap.undefined());
  EXPECT_TRUE(hmap.defined());
  EXPECT_EQ(hmap.toBin(Angle<double>::degrees(1)), 0);
  EXPECT_EQ(hmap.toBin(Angle<double>::degrees(121)), 1);
  EXPECT_EQ(hmap.toBin(Angle<double>::degrees(241)), 2);
  EXPECT_EQ(hmap.toBin(Angle<double>::degrees(361)), 0);
  EXPECT_NEAR(hmap.binToValue(0.00001).degrees(), 0, 1.0);
  EXPECT_NEAR(hmap.binToValue(2.99999999).degrees(), 360.0, 1.0);
  EXPECT_NEAR(hmap.toRightBound(0).radians(), hmap.toLeftBound(1).radians(), 1.0e-6);
  EXPECT_NEAR(hmap.toRightBound(1).radians(), hmap.toLeftBound(2).radians(), 1.0e-6);
  EXPECT_NEAR(hmap.toRightBound(2).radians(), 2.0*M_PI, 1.0e-6);
  EXPECT_NEAR(hmap.toCenter(2).degrees(), 300, 1.0e-6);

  const int count = 7;
  double anglesDeg[count] = {576.2019, 102.1582, 303.6681, 659.3296, 570.3893, 690.8345, 472.1333};
  Array<Angle<double> > angles = transduce(
      Arrayd(count, anglesDeg), trMap([&](double x) {return Angle<double>::degrees(x);}),
      IntoArray<Angle<double>>());

  int bins[count] = {1, 0, 2, 2, 1, 2, 0};
  Arrayi estBins = hmap.assignBins(angles);
  for (int i = 0; i < count; i++) {
    EXPECT_EQ(bins[i], estBins[i]);
  }

  Arrayi hist = Arrayi{2, 2, 3};
  EXPECT_EQ(hist, hmap.countPerBin(angles));

  EXPECT_NEAR(hmap.binSpan(1).minv().degrees(), 120.0, 1.0e-6);
  EXPECT_NEAR(hmap.binSpan(1).maxv().degrees(), 240.0, 1.0e-6);
  EXPECT_EQ(hmap.periodicIndex(3), 0);
  EXPECT_EQ(hmap.periodicIndex(-1), 2);
  EXPECT_EQ(hmap.periodicIndex(-4), 2);

  MDArray2d plotdata = hmap.makePolarPlotData(
      transduce(hist, trMap([&](int x) {return double(x);}), IntoArray<double>()),
      true);
  LineKM rowmap(0, 360, 0.0, plotdata.rows());
  MDArray2d A = plotdata.sliceRow(int(round(rowmap(60))));
  MDArray2d B = plotdata.sliceRow(int(round(rowmap(300))));
  EXPECT_NEAR(2.0, sqrt(A(0, 0)*A(0, 0) + A(0, 1)*A(0, 1)), 1.0e-5);
  EXPECT_NEAR(3.0, sqrt(B(0, 0)*B(0, 0) + B(0, 1)*B(0, 1)), 1.0e-5);
}

TEST(HistogramTest, CustomReference) {

  Angle<double> ref = Angle<double>::radians(2199.10190173919);
  HistogramMap<Angle<double>, true> map = makePolarHistogramMap<double>(9, 1.0, ref);

  Angle<double> marg = Angle<double>::radians(1.0e-2);

  EXPECT_EQ(map.toBin(ref + marg), 1);
  EXPECT_EQ(map.toBin(ref - marg), 0);
}

TEST(HistogramTest, Negative) {
  HistogramMap<Angle<double>, true> map = makePolarHistogramMap<double>(4);
  EXPECT_EQ(map.toBin(Angle<double>::degrees(-1)), 3);
}


TEST(HistogramTest, Slice) {
  HistogramMap<double, false> hist(3, 100.0, 400.0);
  HistogramMap<double, false> h2 = hist.slice(1, 2);
  EXPECT_EQ(1, h2.binCount());
  EXPECT_NEAR(200.0, h2.toLeftBound(0), 1.0e-6);
  EXPECT_NEAR(300.0, h2.toRightBound(0), 1.0e-6);
}


