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
  std::function<HorizontalMotion<double>(BoatSim::ProjectedPosition,Duration<double>)>
    makeConstantFlow(Velocity<double> speed, Angle<double> angle) {
      return [=](BoatSim::ProjectedPosition,Duration<double>) {
        return HorizontalMotion<double>::polar(speed, angle);
      };
  }

  const Angle<double> tol = Angle<double>::degrees(5.0);
}

TEST(BoatSimTest, SimLimit) {
  BoatCharacteristics ch;
  auto windfun = makeConstantFlow(Velocity<double>::metersPerSecond(8),
                                  Angle<double>::degrees(0));
  auto currentfun = makeConstantFlow(Velocity<double>::knots(0.1),
                                  Angle<double>::degrees(90));

  auto fun = BoatSim::makePiecewiseTwaFunction(
      Array<Duration<double> >{Duration<double>::hours(3.0)},
      Array<Angle<double> >{Angle<double>::degrees(129)});
  BoatSim simulator(windfun, currentfun, ch, fun);

  Array<BoatSim::FullState> states = simulator.simulate(Duration<double>::seconds(30.0),
      Duration<double>::seconds(1.0), 20);

  EXPECT_LE(30, std::abs(states.first().windAngleWrtWater.degrees()));
  EXPECT_NEAR(129, states.last().windAngleWrtWater.degrees(), tol.degrees());
}

TEST(BoatSimTest, SimDirectionChange) {
  BoatCharacteristics ch;
  auto windfun = makeConstantFlow(Velocity<double>::metersPerSecond(8),
                                  Angle<double>::degrees(0));
  auto currentfun = makeConstantFlow(Velocity<double>::knots(0.1),
                                  Angle<double>::degrees(90));

  auto fun = BoatSim::makePiecewiseTwaFunction(
    Array<Duration<double> >::fill(2, Duration<double>::minutes(2.0)),
        Array<Angle<double> >{
            Angle<double>::degrees(129),
            Angle<double>::degrees(199)});

  BoatSim simulator(windfun, currentfun, ch, fun);

  Array<BoatSim::FullState> states = simulator.simulate(Duration<double>::minutes(3.99),
      Duration<double>::seconds(1.0), 20);


  EXPECT_NEAR(129, states[states.middle()].windAngleWrtWater.degrees(), tol.degrees());
  EXPECT_NEAR(199, states.last().windAngleWrtWater.degrees(), tol.degrees());
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
  auto fun = BoatSim::makePiecewiseTwaFunction(
      Array<Duration<double> >{Duration<double>::hours(3.0)},
      Array<Angle<double> >{Angle<double>::degrees(90)});

  BoatSim simulator(windfun, currentfun, ch, fun);

  Array<BoatSim::FullState> states = simulator.simulate(Duration<double>::seconds(90.0),
      Duration<double>::seconds(1.0), 20);

  auto last = states.last();
  EXPECT_NEAR(90, last.windAngleWrtWater.degrees(), tol.degrees());
  EXPECT_NEAR(last.boatOrientation.degrees(), 0.0, tol.degrees());
  EXPECT_NEAR(last.boatSpeedThroughWater.knots(), 1.3 + last.boatMotion.norm().knots(), 0.1);
}


