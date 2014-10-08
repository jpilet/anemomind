/*
 *  Created on: 2014-09-05
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/polar/BasicPolar.h>
#include <server/common/Histogram.h>

using namespace sail;

namespace {
  Array<PolarPoint> makePts1() {
    return Array<PolarPoint>::args(PolarPoint(
      Velocity<double>::metersPerSecond(6.3),
      Angle<double>::degrees(47),
      Velocity<double>::knots(4.1)));
  }

  Array<PolarPoint> makePts3() {
    return Array<PolarPoint>::args(
      PolarPoint(
        Velocity<double>::metersPerSecond(6.3),
        Angle<double>::degrees(47),
        Velocity<double>::knots(4.1)),

      PolarPoint(
        Velocity<double>::metersPerSecond(6.3),
        Angle<double>::degrees(47),
        Velocity<double>::knots(5.2)),

      PolarPoint(
        Velocity<double>::metersPerSecond(14),
        Angle<double>::degrees(359),
        Velocity<double>::knots(7.0)));
  }


   BasicPolar::TwsHist makeTws(int binCount) {
     return BasicPolar::TwsHist(binCount,
             Velocity<double>::metersPerSecond(0),
             Velocity<double>::metersPerSecond(25));
   }


}

TEST(BasicPolarTest, Empty) {
  BasicPolar polar(makeTws(9), makePolarHistogramMap(3), Array<PolarPoint>());
  EXPECT_EQ(0, polar.pointCount());
  EXPECT_EQ(9, polar.twsHist().binCount());
  BasicPolar trimmed = polar.trim();
  EXPECT_EQ(0, trimmed.pointCount());
  EXPECT_EQ(0, trimmed.twsHist().binCount());

}

TEST(BasicPolarTest, SimpleTest) {
  BasicPolar::TwsHist twsHist = makeTws(24);
  PolarSlice::TwaHist twaHist = makePolarHistogramMap(3);
  Array<PolarPoint> pts = makePts1();
  BasicPolar polar(twsHist, twaHist, pts);
  EXPECT_EQ(1, polar.pointCount());
  EXPECT_EQ(0, polar.slices()[5].pointCount());
  EXPECT_EQ(1, polar.slices()[6].pointCount());
  EXPECT_EQ(0, polar.slices()[7].pointCount());
  BasicPolar trimmed = polar.trim();
  EXPECT_EQ(1, trimmed.slices().size());
  EXPECT_EQ(1, trimmed.pointCount());
  EXPECT_NEAR(4.1, trimmed.slices().first().lookUpBoatSpeed(0, 1.0).knots(), 1.0e-6);
  EXPECT_NEAR(4.1, trimmed.slices().first().lookUpBoatSpeed(0, 0.8).knots(), 1.0e-6);
  EXPECT_NEAR(0.0, trimmed.slices().first().lookUpBoatSpeedOr0(1, 0.8).knots(), 1.0e-6);
  EXPECT_NEAR(4.1, trimmed.slices().first().lookUpBoatSpeed(Angle<double>::degrees(46), 0.8).knots(), 1.0e-6);
}

TEST(BasicPolarTest, Pts3) {
  BasicPolar::TwsHist twsHist = makeTws(24);
  PolarSlice::TwaHist twaHist = makePolarHistogramMap(3);
  Array<PolarPoint> pts = makePts3();
  BasicPolar polar = BasicPolar(twsHist, twaHist, pts).trim();


  PolarSlice strongWind = polar.slices().last();
  PolarSlice gentleWind = polar.slices().first();

  EXPECT_EQ(3, polar.pointCount());
  EXPECT_EQ(1, strongWind.pointCount());
  EXPECT_EQ(2, gentleWind.pointCount());

  EXPECT_NEAR(5.2, gentleWind.lookUpBoatSpeedOr0(Angle<double>::degrees(47), 0.8).knots(), 1.0e-6);
  EXPECT_NEAR(0.0, gentleWind.lookUpBoatSpeedOr0(Angle<double>::degrees(359), 0.8).knots(), 1.0e-6);

  EXPECT_NEAR(0.0, strongWind.lookUpBoatSpeedOr0(Angle<double>::degrees(47), 0.8).knots(), 1.0e-6);
  EXPECT_NEAR(7.0, strongWind.lookUpBoatSpeedOr0(Angle<double>::degrees(359), 0.8).knots(), 1.0e-6);
}
