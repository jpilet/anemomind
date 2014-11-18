/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/BoatSim.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <iostream>

namespace {
  using namespace sail;

  std::function<HorizontalMotion<double>(Length<double>, Length<double>,Duration<double>)>
    makeConstantFlow(Velocity<double> speed, Angle<double> angle) {
      return [=](Length<double>, Length<double>,Duration<double>) {
        return HorizontalMotion<double>::polar(speed, angle);
      };
  }

  Arrayd getTimes(Array<BoatSimulator::FullBoatState> states) {
    return states.map<double>([=](BoatSimulator::FullBoatState s) {
      return s.time.seconds();
    });
  }


  void plotAngles(const char *title, Array<BoatSimulator::FullBoatState> states,
      std::function<Angle<double>(BoatSimulator::FullBoatState)> fun) {
    GnuplotExtra plot;
    plot.set_title(title);
    plot.set_style("lines");
    plot.plot_xy(getTimes(states),
        states.map<Angle<double> >(fun).map<double>([&](Angle<double> x)
            {return x.degrees();}));
    plot.show();
  }

  void plotSpeed(const char *title, Array<BoatSimulator::FullBoatState> states,
      std::function<Velocity<double>(BoatSimulator::FullBoatState)> fun) {
    GnuplotExtra plot;
    plot.set_title(title);
    plot.set_style("lines");
    plot.plot_xy(getTimes(states),
        states.map<Velocity<double> >(fun).map<double>([&](Velocity<double> x)
            {return x.knots();}));
    plot.show();
  }

  void plotTrajectory(Array<BoatSimulator::FullBoatState> states) {
    GnuplotExtra plot;
    plot.set_title("Trajectory (meters)");
    plot.set_style("lines");
    plot.plot_xy(
        states.map<double>([&](BoatSimulator::FullBoatState x) {return x.x.meters();}),
        states.map<double>([&](BoatSimulator::FullBoatState x) {return x.y.meters();})
    );
    plot.show();
  }

  void ex0() {
    Duration<double> dur = Duration<double>::minutes(0.5);

    BoatCharacteristics ch;
    auto windfun = makeConstantFlow(Velocity<double>::metersPerSecond(8),
                                    Angle<double>::degrees(0));
    auto currentfun = makeConstantFlow(Velocity<double>::knots(0.1),
                                    Angle<double>::degrees(90));

    BoatSimulator::TwaDirective d1(dur,
                                  Angle<double>::degrees(129));
    BoatSimulator::TwaDirective d2(dur,
                                  Angle<double>::degrees(199));

    auto twadir = Array<BoatSimulator::TwaDirective>::args(d1, d2);
    BoatSimulator simulator(windfun, currentfun, ch, twadir);

    Array<BoatSimulator::FullBoatState> states = simulator.simulate(1.99*dur,
        Duration<double>::seconds(0.05), 1);


    plotAngles("Rudder angle (degrees)", states, [](const BoatSimulator::FullBoatState &x) {return x.rudderAngle;});
    plotAngles("Boat orientation (degrees)", states, [](const BoatSimulator::FullBoatState &x) {return x.boatOrientation;});
    plotSpeed("Boat speed through water (knots)", states, [](const BoatSimulator::FullBoatState &x) {return x.boatSpeedThroughWater;});
    plotTrajectory(states);
  }
}

int main(int argc, const char **argv) {
  ex0();
  return 0;
}


