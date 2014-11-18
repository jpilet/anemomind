/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/BoatSim.h>
#include <iostream>
#include <server/common/string.h>

using namespace sail;

namespace {
  std::function<HorizontalMotion<double>(Length<double>, Length<double>,Duration<double>)>
    makeConstantFlow(Velocity<double> speed, Angle<double> angle) {
      return [=](Length<double>, Length<double>,Duration<double>) {
        return HorizontalMotion<double>::polar(speed, angle);
      };
  }
}

TEST(BoatSimTest, SimLimit) {
  BoatCharacteristics ch;
  auto windfun = makeConstantFlow(Velocity<double>::metersPerSecond(8),
                                  Angle<double>::degrees(0));
  auto currentfun = makeConstantFlow(Velocity<double>::knots(0.1),
                                  Angle<double>::degrees(90));

  BoatSimulator::TwaDirective d(Duration<double>::hours(3.0), Angle<double>::degrees(129));
  auto twadir = Array<BoatSimulator::TwaDirective>::args(d);
  BoatSimulator simulator(windfun, currentfun, ch, twadir);

  Array<BoatSimulator::FullBoatState> states = simulator.simulate(Duration<double>::seconds(30.0),
      Duration<double>::seconds(1.0), 20);

  EXPECT_LE(30, std::abs(states.first().twaWater.degrees()));
  EXPECT_NEAR(129, states.last().twaWater.degrees(), 1.0);
}


