/*
 *  Created on: 2014-09-04
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/PolarHistogram.h>

using namespace sail;

/*

/*
 *
  PolarHistogramMap(int binCount,
      double refBinIndex = 0,
      Angle<double> refAngle = Angle<double>::radians(0));
  int toBin(Angle<double> value) const;
  Angle<double> binIndexToAngle(double binIndex) const;
  Angle<double> toLeftBound(int binIndex) const;
  Angle<double> toRightBound(int binIndex) const;
  Angle<double> toCenter(int binIndex) const;
  Arrayi countPerBin(Array<Angle<double> > angles) const;
  Arrayi assignBins(Array<Angle<double> > values) const;
  int periodicIndex(int index) const;
  Span<Angle<double> > binSpan(int binIndex) const;

  int binCount() const {return _binCount;}
  bool defined() const {return _binCount > 0;}
  bool undefined() const {return !defined();}
 */
TEST(PolarHistogramTest, Undefined) {
  PolarHistogramMap map;
  EXPECT_FALSE(map.defined());
  EXPECT_TRUE(map.undefined());
}

 TEST(PolarHistogramTest, BasicTest) {
  PolarHistogramMap map(3);
  EXPECT_FALSE(map.undefined());
  EXPECT_TRUE(map.defined());
  EXPECT_EQ(map.toBin(Angle<double>::degrees(1)), 0);
  EXPECT_EQ(map.toBin(Angle<double>::degrees(121)), 1);
  EXPECT_EQ(map.toBin(Angle<double>::degrees(241)), 2);
  EXPECT_EQ(map.toBin(Angle<double>::degrees(361)), 0);
  EXPECT_NEAR(map.binIndexToAngle(0.00001).degrees(), 0, 1.0);
  EXPECT_NEAR(map.binIndexToAngle(2.99999999).degrees(), 360.0, 1.0);
  EXPECT_NEAR(map.toRightBound(0).radians(), map.toLeftBound(1).radians(), 1.0e-6);
  EXPECT_NEAR(map.toRightBound(1).radians(), map.toLeftBound(2).radians(), 1.0e-6);
  EXPECT_NEAR(map.toRightBound(2).radians(), 2.0*M_PI, 1.0e-6);
  EXPECT_NEAR(map.toCenter(2).degrees(), 300, 1.0e-6);
}


