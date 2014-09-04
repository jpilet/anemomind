/*
 *  Created on: 2014-09-04
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/PolarHistogram.h>

using namespace sail;

TEST(PolarHistogramTest, Undefined) {
  PolarHistogramMap map;
  EXPECT_FALSE(map.defined());
  EXPECT_TRUE(map.undefined());
}

 TEST(PolarHistogramTest, BasicTest) {
  PolarHistogramMap map(3);
  EXPECT_FALSE(map.undefined());
  EXPECT_TRUE(map.defined());
  EXPECT_EQ(map.angleToBin(Angle<double>::degrees(1)), 0);
  EXPECT_EQ(map.angleToBin(Angle<double>::degrees(121)), 1);
  EXPECT_EQ(map.angleToBin(Angle<double>::degrees(241)), 2);
  EXPECT_EQ(map.angleToBin(Angle<double>::degrees(361)), 0);
  EXPECT_NEAR(map.binIndexToAngle(0.00001).degrees(), 0, 1.0);
  EXPECT_NEAR(map.binIndexToAngle(2.99999999).degrees(), 360.0, 1.0);
  EXPECT_NEAR(map.toRightBound(0).radians(), map.toLeftBound(1).radians(), 1.0e-6);
  EXPECT_NEAR(map.toRightBound(1).radians(), map.toLeftBound(2).radians(), 1.0e-6);
  EXPECT_NEAR(map.toRightBound(2).radians(), 2.0*M_PI, 1.0e-6);
  EXPECT_NEAR(map.toCenter(2).degrees(), 300, 1.0e-6);

  const int count = 7;
  double anglesDeg[count] = {576.2019, 102.1582, 303.6681, 659.3296, 570.3893, 690.8345, 472.1333};
  Array<Angle<double> > angles = Arrayd(count, anglesDeg)
      .map<Angle<double> >([&](double x) {return Angle<double>::degrees(x);});

  int bins[count] = {1, 0, 2, 2, 1, 2, 0};
  Arrayi estBins = map.assignBins(angles);
  for (int i = 0; i < count; i++) {
    EXPECT_EQ(bins[i], estBins[i]);
  }

  Arrayi hist = Arrayi::args(2, 2, 3);
  EXPECT_EQ(hist, map.countPerBin(angles));

  EXPECT_NEAR(map.binSpan(1).minv().degrees(), 120.0, 1.0e-6);
  EXPECT_NEAR(map.binSpan(1).maxv().degrees(), 240.0, 1.0e-6);
  EXPECT_EQ(map.periodicIndex(3), 0);
  EXPECT_EQ(map.periodicIndex(-1), 2);
  EXPECT_EQ(map.periodicIndex(-4), 2);
}

TEST(PolarHistogramTest, CustomReference) {

  Angle<double> ref = Angle<double>::radians(2199.10190173919);
  PolarHistogramMap map(9, 1.0, ref);

  Angle<double> marg = Angle<double>::radians(1.0e-2);

  EXPECT_EQ(map.angleToBin(ref + marg), 1);
  EXPECT_EQ(map.angleToBin(ref - marg), 0);
}

TEST(PolarHistogramTest, Negative) {
  PolarHistogramMap map(4);
  EXPECT_EQ(map.angleToBin(Angle<double>::degrees(-1)), 3);
}


