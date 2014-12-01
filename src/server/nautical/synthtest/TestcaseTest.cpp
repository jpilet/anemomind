/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Testcases to test the Testcase class :-)
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/Testcase.h>

using namespace sail;

TEST(TestcaseTest, Test1) {
  HorizontalMotion<double> wind = HorizontalMotion<double>::polar(
      Velocity<double>::metersPerSecond(7.3),
      Angle<double>::degrees(129));

  HorizontalMotion<double> current = HorizontalMotion<double>::polar(
      Velocity<double>::knots(1.3),
      Angle<double>::degrees(77));


  TimeStamp timeoffset = TimeStamp::UTC(2012, 10, 13, 5, 34, 23);

  auto pos = GeographicPosition<double>(Angle<double>::degrees(30),
                                        Angle<double>::degrees(45));
  GeographicReference georef(pos);

  Testcase::FlowFun windfun = Testcase::constantFlowFun(wind);
  Testcase::FlowFun currentfun = Testcase::constantFlowFun(wind);

  std::default_random_engine e;
  Testcase tc(e, georef, timeoffset, windfun, currentfun,
    Array<Testcase::BoatSimDirs>(0));

  EXPECT_NEAR(tc.toLocalTime(timeoffset).seconds(), 0.0, 1.0e-6);
  auto m = tc.geoRef().map(pos);
  EXPECT_NEAR(m[0].meters(), 0.0, 1.0e-4);
  EXPECT_NEAR(m[1].meters(), 0.0, 1.0e-4);

}


