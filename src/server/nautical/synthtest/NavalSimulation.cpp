/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavalSimulation.h"
#include <server/common/PhysicalQuantityIO.h>
#include <server/nautical/synthtest/Flow.h>
#include <iostream>
#include <server/common/PhysicalQuantityIO.h>
#include <server/common/logging.h>
#include <server/common/Functional.h>

namespace sail {

using namespace NavCompat;

NavalSimulation::FlowFun NavalSimulation::constantFlowFun(HorizontalMotion<double> m) {
  return [=](const ProjectedPosition &pos, Duration<double> dur) {
    return m;
  };
}

BoatSimulationSpecs::BoatSimulationSpecs(BoatCharacteristics ch, Array<TwaDirective> dirs,
  CorruptedBoatState::CorruptorSet corruptors,
  Nav::Id boatId, Duration<double> samplingPeriod, int stepsPerSample) :
    _ch(ch),
    _dirs(dirs),
    _corruptors(corruptors),
    _indexer(ProportionateIndexer(dirs.size(),
        [=](int index) {return dirs[index].duration.seconds();})),
        _boatId(boatId), _samplingPeriod(samplingPeriod),
        _stepsPerSample(stepsPerSample) {}


Angle<double> BoatSimulationSpecs::twa(Duration<double> dur) const {
  auto result = _indexer.get(dur.seconds());
  return _dirs[result.index].interpolate(result.localX);
}

NavalSimulation::NavalSimulation(std::default_random_engine &e,
         GeographicReference geoRef,
         TimeStamp timeOffset,
         FlowFun wind,
         FlowFun current,
         Array<BoatSimulationSpecs> specs) :
         _geoRef(geoRef),
         _simulationStartTime(timeOffset),
         _wind(wind),
         _current(current), _boatData(specs.size()) {
  for (int i = 0; i < specs.size(); i++) {
    auto sp = specs[i];
    BoatSim sim(wind, current, sp.characteristics(),
        [=](Duration<double> dur) {return sp.twa(dur);});
    Array<BoatSim::FullState> states = sim.simulate(sp.duration(),
        sp.samplingPeriod(), sp.stepsPerSample());
    _boatData[i] = makeBoatData(sp, states, e);
  }
}

NavalSimulation::SimulatedCalibrationResults NavalSimulation::BoatData::evaluateFitness(
    const CorrectorFunction &corr) const {
  int count = _states.size();
  Array<HorizontalMotion<double> > estWind(count), estCurrent(count);
  Spani span(0, count);
  auto navs = toArray(map(span, [&](int i) {return _states[i].nav();}));
  auto cnavs = corr(fromNavs(navs));
  for (auto i: span) {
    estWind[i] = cnavs[i].trueWindOverGround();
    estCurrent[i] = cnavs[i].trueCurrentOverGround();
  }
  return SimulatedCalibrationResults(
          SimulatedMotionResults(trueWindOverGround(), estWind),
          SimulatedMotionResults(trueCurrentOverGround(), estCurrent));
}


NavalSimulation::SimulatedCalibrationResults NavalSimulation::BoatData::evaluateNoCalibration() const {
  auto corrFun = CorrectorObject(Corrector<double>());
  return evaluateFitness(corrFun);
}


NavalSimulation::SimulatedCalibrationResults
  NavalSimulation::BoatData::evaluateFitness(Array<HorizontalMotion<double> > estimatedTrueWind,
          Array<HorizontalMotion<double> > estimatedTrueCurrent) const {
  return SimulatedCalibrationResults(
      SimulatedMotionResults(trueWindOverGround(), estimatedTrueWind),
      SimulatedMotionResults(trueCurrentOverGround(), estimatedTrueCurrent));
}

NavalSimulation::SimulatedCalibrationResults NavalSimulation::BoatData::evaluateFitness(
    Array<CalibratedNav<double> > cnavs) const {
    return evaluateFitness(map(cnavs, [&](const CalibratedNav<double> &x) {
      return x.trueWindOverGround();
    }), map(cnavs, [&] (const CalibratedNav<double> &x) {
      return x.trueCurrentOverGround();
    }));
}



Array<HorizontalMotion<double> > NavalSimulation::BoatData::trueWindOverGround() const {
  return map(_states, [=] (const CorruptedBoatState &s) {
    return s.trueState().trueWind;
  });
}
Array<HorizontalMotion<double> > NavalSimulation::BoatData::trueCurrentOverGround() const {
  return map(_states, [=] (const CorruptedBoatState &s) {
    return s.trueState().trueCurrent;
  });
}

void NavalSimulation::BoatData::plot() const {
  BoatSim::makePlots(
      map(_states, [=](const CorruptedBoatState &x) {
        return x.trueState();
      }));
}


NavalSimulation::BoatData NavalSimulation::makeBoatData(BoatSimulationSpecs &specs,
    Array<BoatSim::FullState> states, std::default_random_engine &e) const {
  int count = states.size();
  Array<CorruptedBoatState> dst(count);
  auto c = specs.corruptors();
  for (int i = 0; i < count; i++) {
    auto &state = states[i];
    Nav dstnav;
    dstnav.setAwa(c.awa.corrupt(state.awa(), e));
    dstnav.setAws(c.aws.corrupt(state.apparentWind().norm(), e));
    dstnav.setMagHdg(c.magHdg.corrupt(state.boatOrientation, e));
    dstnav.setGpsBearing(c.gpsBearing.corrupt(state.boatMotion.angle(), e));
    dstnav.setGpsSpeed(c.gpsSpeed.corrupt(state.boatMotion.norm(), e));
    dstnav.setWatSpeed(c.watSpeed.corrupt(state.boatMotionThroughWater.norm(), e));
    dstnav.setTime(fromLocalTime(state.time));
    dstnav.setGeographicPosition(geoRef().unmap(state.pos));
    dstnav.setBoatId(specs.boatId());
    dst[i] = CorruptedBoatState(state, dstnav);
  }
  return BoatData(specs, dst);
}




NavalSimulation makeNavSimConstantFlow() {
  std::default_random_engine e;

  GeographicReference geoRef(GeographicPosition<double>(
      Angle<double>::degrees(30),
      Angle<double>::degrees(29)));
  TimeStamp simulationStartTime = TimeStamp::UTC(2014, 12, 15, 12, 06, 29);

  auto wind = NavalSimulation::constantFlowFun(
     HorizontalMotion<double>::polar(Velocity<double>::metersPerSecond(10.8),
                                     Angle<double>::degrees(306)));
  auto current = NavalSimulation::constantFlowFun(
     HorizontalMotion<double>::polar(Velocity<double>::knots(0.5),
                                     Angle<double>::degrees(49)));


  Array<BoatSimulationSpecs::TwaDirective> dirs(12);
  for (int i = 0; i < 12; i++) {
    dirs[i] = BoatSimulationSpecs::TwaDirective::constant(
        Duration<double>::minutes(2.0),
        Angle<double>::degrees((i + 2)*67.0));
  }

  CorruptedBoatState::CorruptorSet corruptors2;
    corruptors2.awa = CorruptedBoatState::Corruptor<Angle<double> >::offset(
        Angle<double>::degrees(9.8));
    corruptors2.magHdg = CorruptedBoatState::Corruptor<Angle<double> >::offset(
        Angle<double>::degrees(-3.3));
    corruptors2.aws = CorruptedBoatState::Corruptor<Velocity<double> >(1.09, Velocity<double>::knots(0.3));
    corruptors2.watSpeed = CorruptedBoatState::Corruptor<Velocity<double> >(0.94, Velocity<double>::knots(-0.2));

  Array<BoatSimulationSpecs> specs(2);
  specs[0] = BoatSimulationSpecs(BoatCharacteristics(),
      dirs,
      CorruptedBoatState::CorruptorSet());
  specs[1] = BoatSimulationSpecs(BoatCharacteristics(),
      dirs,
      corruptors2);


  return NavalSimulation(e, geoRef,
           simulationStartTime,
           wind,
           current,
           specs
           );
}

namespace {
NavalSimulation makeNavSimUpwindDownwind(int count) {
    std::default_random_engine e;

    GeographicReference geoRef(GeographicPosition<double>(
        Angle<double>::degrees(30),
        Angle<double>::degrees(29)));
    TimeStamp simulationStartTime = TimeStamp::UTC(2014, 12, 15, 12, 06, 29);

    auto wind = (Flow::constant(Velocity<double>::metersPerSecond(6),
        Velocity<double>::metersPerSecond(-2)) +
            Flow(
            Flow::spatiallyChangingVelocity(
                      Velocity<double>::metersPerSecond(1.3),
                      Angle<double>::radians(3234.234),
                      Length<double>::meters(30),
                      Angle<double>::radians(34.4)),
            Flow::spatiallyChangingVelocity(
                      Velocity<double>::metersPerSecond(1.0),
                      Angle<double>::radians(54.2),
                      Length<double>::meters(51),
                      Angle<double>::radians(12.4)))).asFunction();
    auto current = (Flow::constant(Velocity<double>::metersPerSecond(0.1),
          Velocity<double>::metersPerSecond(-0.2)) +
              Flow(
              Flow::spatiallyChangingVelocity(
                        Velocity<double>::knots(0.29),
                        Angle<double>::radians(99),
                        Length<double>::meters(4),
                        Angle<double>::radians(234.912)),
              Flow::spatiallyChangingVelocity(
                        Velocity<double>::knots(0.1),
                        Angle<double>::radians(98.66),
                        Length<double>::meters(9),
                        Angle<double>::radians(2996.33)))).asFunction();

    Array<BoatSimulationSpecs::TwaDirective> dirs(2*count);
    for (int i = 0; i < count; i++) {
      int sign = 2*(i % 2) - 1;
      // Upwind
      dirs[i + 0] = BoatSimulationSpecs::TwaDirective::constant(
          Duration<double>::minutes(2.0),
          Angle<double>::degrees(sign*45));

      // Downwind
      dirs[i + count] = BoatSimulationSpecs::TwaDirective::constant(
          Duration<double>::minutes(2.0),
          Angle<double>::degrees(180 + sign*20));
    }

    CorruptedBoatState::CorruptorSet corruptors1;
      corruptors1.awa = CorruptedBoatState::Corruptor<Angle<double> >::offset(
          Angle<double>::degrees(-4));
      corruptors1.magHdg = CorruptedBoatState::Corruptor<Angle<double> >::offset(
          Angle<double>::degrees(-9));
      corruptors1.aws = CorruptedBoatState::Corruptor<Velocity<double> >(1.12, Velocity<double>::knots(-0.5));
      corruptors1.watSpeed = CorruptedBoatState::Corruptor<Velocity<double> >(1.3, Velocity<double>::knots(0.8));

      CorruptedBoatState::CorruptorSet corruptors2;
        corruptors2.awa = CorruptedBoatState::Corruptor<Angle<double> >::offset(
            Angle<double>::degrees(-14));
        corruptors2.magHdg = CorruptedBoatState::Corruptor<Angle<double> >::offset(
            Angle<double>::degrees(-1));
        corruptors2.aws = CorruptedBoatState::Corruptor<Velocity<double> >(1.2, Velocity<double>::knots(0.0));
        corruptors2.watSpeed = CorruptedBoatState::Corruptor<Velocity<double> >(1.0, Velocity<double>::knots(-0.7));


    Array<BoatSimulationSpecs> specs(2);
    specs[0] = BoatSimulationSpecs(BoatCharacteristics(),
        dirs,
        corruptors1);
    specs[1] = BoatSimulationSpecs(BoatCharacteristics(),
        dirs,
        corruptors2);


    return NavalSimulation(e, geoRef,
             simulationStartTime,
             wind,
             current,
             specs
             );
  }
}

NavalSimulation makeNavSimUpwindDownwind() {
  return makeNavSimUpwindDownwind(6);
}

NavalSimulation makeNavSimUpwindDownwindLong() {
  return makeNavSimUpwindDownwind(18);
}


std::ostream &operator<< (std::ostream &s, const NavalSimulation::SimulatedCalibrationResults &e) {
  s << "FlowErrors(\n";
  s << "  wind    = " << e.wind().error() << "\n";
  s << "  current = " << e.current().error() << "\n";
  s << ")\n";
  return s;
}


namespace {
  Angle<double> getMainDir(Duration<double> time, Duration<double> legDur) {
    double side = positiveMod(time.hours(), 2*legDur.hours());
    if (side > legDur.hours()) {
      return Angle<double>::degrees(180);
    }
    return Angle<double>::degrees(0);
  }
}

Array<CorruptedBoatState::CorruptorSet> makeCorruptorSets001() {
  CorruptedBoatState::CorruptorSet corruptorSet;
  corruptorSet.awa = CorruptedBoatState::Corruptor<Angle<double> >::offset(
      Angle<double>::degrees(-14));
  corruptorSet.magHdg = CorruptedBoatState::Corruptor<Angle<double> >::offset(
      Angle<double>::degrees(-1));
  corruptorSet.aws = CorruptedBoatState::Corruptor<Velocity<double> >(1.2, Velocity<double>::knots(0.0));
  corruptorSet.watSpeed = CorruptedBoatState::Corruptor<Velocity<double> >(1.0, Velocity<double>::knots(-0.7));
  return Array<CorruptedBoatState::CorruptorSet>{corruptorSet};
}



Array<BoatSimulationSpecs::TwaDirective> makeTwaDirectivesSub(
  Duration<double> legDur, Duration<double> totalDur, Duration<double> tackDur) {

  int tackCount = int(floor(totalDur/tackDur));
  LOG(INFO) << stringFormat("Tack count: %d", tackCount);

  Array<BoatSimulationSpecs::TwaDirective> dirs(tackCount);
  for (int i = 0; i < tackCount; i++) {
    Angle<double> tackAngle = double((2*(i % 2) - 1))*Angle<double>::degrees(45);
    dirs[i] = BoatSimulationSpecs::TwaDirective::constant(tackDur,
        tackAngle + getMainDir(double(i)*tackDur, legDur));
  }
  return dirs;
}

Array<BoatSimulationSpecs::TwaDirective> makeTwaDirectives001() {
  Duration<double> legDur = Duration<double>::minutes(10.0);
  Duration<double> totalDur = Duration<double>::hours(1);
  Duration<double> tackDur = Duration<double>::minutes(1);

  return makeTwaDirectivesSub(legDur, totalDur, tackDur);
}

Array<BoatSimulationSpecs::TwaDirective> makeTwaDirectives002() {
  Duration<double> legDur = Duration<double>::minutes(100.0);
  Duration<double> totalDur = Duration<double>::days(2);
  Duration<double> tackDur = Duration<double>::minutes(10);

  return makeTwaDirectivesSub(legDur, totalDur, tackDur);
}

Array<BoatSimulationSpecs> makeSpecs001(int stepsPerSample, Array<BoatSimulationSpecs::TwaDirective> dirs) {
  auto sets = makeCorruptorSets001();

  int n = sets.size();
  Array<BoatSimulationSpecs> specs(n);
  for (int i = 0; i < n; i++) {
    specs[i] = BoatSimulationSpecs(BoatCharacteristics(),
          dirs,
          sets[i], Nav::debuggingBoatId(),
          Duration<double>::seconds(1.0),
          stepsPerSample);
  }
  return specs;
}

namespace {

  GeographicReference makeGeoRef() {
    return GeographicPosition<double>(
        Angle<double>::degrees(30),
        Angle<double>::degrees(29));
  }

  TimeStamp makeStartTime() {
    return TimeStamp::UTC(2014, 12, 15, 12, 06, 29);
  }


}




}
