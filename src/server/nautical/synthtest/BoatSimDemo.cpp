/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/BoatSim.h>
#include <server/common/string.h>
#include <iostream>

namespace {
  using namespace sail;

  std::function<HorizontalMotion<double>(BoatSim::ProjectedPosition,Duration<double>)>
    makeConstantFlow(Velocity<double> speed, Angle<double> angle) {
      return [=](BoatSim::ProjectedPosition,Duration<double>) {
        return HorizontalMotion<double>::polar(speed, angle);
      };
  }


}

int main(int argc, const char **argv) {
  Duration<double> dur = Duration<double>::minutes(0.5);

  BoatCharacteristics ch;
  auto windfun = makeConstantFlow(Velocity<double>::metersPerSecond(8),
                                  Angle<double>::degrees(0));
  auto currentfun = makeConstantFlow(Velocity<double>::knots(0.1),
                                  Angle<double>::degrees(90));


  auto fun = BoatSim::makePiecewiseTwaFunction(Array<Duration<double> >{dur, dur},
      Array<Angle<double> >{Angle<double>::degrees(129), Angle<double>::degrees(199)});

  BoatSim simulator(windfun, currentfun, ch, fun);

  Array<BoatSim::FullState> states = simulator.simulate(1.99*dur,
      Duration<double>::seconds(0.05), 1);

  BoatSim::makePlots(states);
  return 0;
}


