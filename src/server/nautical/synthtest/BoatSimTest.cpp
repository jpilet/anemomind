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

  const Angle<double> tol = Angle<double>::degrees(3.0);
}

TEST(BoatSimTest, SimLimit) {
  BoatCharacteristics ch;
  auto windfun = makeConstantFlow(Velocity<double>::metersPerSecond(8),
                                  Angle<double>::degrees(0));
  auto currentfun = makeConstantFlow(Velocity<double>::knots(0.1),
                                  Angle<double>::degrees(90));

  auto fun = BoatSimulator::makePiecewiseTwaFunction(
      Array<Duration<double> >::args(Duration<double>::hours(3.0)),
      Array<Angle<double> >::args(Angle<double>::degrees(129)));
  BoatSimulator simulator(windfun, currentfun, ch, fun);

  Array<BoatSimulator::FullBoatState> states = simulator.simulate(Duration<double>::seconds(30.0),
      Duration<double>::seconds(1.0), 20);

  EXPECT_LE(30, std::abs(states.first().twaWater.degrees()));
  EXPECT_NEAR(129, states.last().twaWater.degrees(), tol.degrees());
}

TEST(BoatSimTest, SimDirectionChange) {
  BoatCharacteristics ch;
  auto windfun = makeConstantFlow(Velocity<double>::metersPerSecond(8),
                                  Angle<double>::degrees(0));
  auto currentfun = makeConstantFlow(Velocity<double>::knots(0.1),
                                  Angle<double>::degrees(90));

  auto fun = BoatSimulator::makePiecewiseTwaFunction(
    Array<Duration<double> >::fill(2, Duration<double>::minutes(2.0)),
        Array<Angle<double> >::args(
            Angle<double>::degrees(129),
            Angle<double>::degrees(199)));

  BoatSimulator simulator(windfun, currentfun, ch, fun);

  Array<BoatSimulator::FullBoatState> states = simulator.simulate(Duration<double>::minutes(3.99),
      Duration<double>::seconds(1.0), 20);


  EXPECT_NEAR(129, states[states.middle()].twaWater.degrees(), tol.degrees());
  EXPECT_NEAR(199, states.last().twaWater.degrees(), tol.degrees());
}

// Inspired by the test CorrectorTest::BeamReachWithCurrent, but not identical
TEST(BoatSimTest, CheckAllValues) {

  BoatCharacteristics ch;

  // Wind is blowing from east
  auto windfun = makeConstantFlow(Velocity<double>::metersPerSecond(12),
                                  Angle<double>::degrees(270));

  // Current coming from north
  auto currentfun = makeConstantFlow(Velocity<double>::knots(1.3),
                                  Angle<double>::degrees(180));

  // We sail north, so the true wind angle is 90 degrees.
  auto fun = BoatSimulator::makePiecewiseTwaFunction(
      Array<Duration<double> >::args(Duration<double>::hours(3.0)),
      Array<Angle<double> >::args(Angle<double>::degrees(90)));

  BoatSimulator simulator(windfun, currentfun, ch, fun);

  Array<BoatSimulator::FullBoatState> states = simulator.simulate(Duration<double>::seconds(90.0),
      Duration<double>::seconds(1.0), 20);

  auto last = states.last();
  EXPECT_NEAR(90, last.twaWater.degrees(), tol.degrees());
  EXPECT_NEAR(last.boatOrientation.degrees(), 0.0, 5.0);
}


