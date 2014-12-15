/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavalSimulation.h"

namespace sail {

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
    BoatSim sim(wind, current, sp.characteristics(), [=](Duration<double> dur) {return sp.twa(dur);});
    Array<BoatSim::FullState> states = sim.simulate(sp.duration(), sp.samplingPeriod(), sp.stepsPerSample());
    _boatData[i] = makeBoatData(sp, states, e);
  }
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

NavalSimulation makeNavSim001() {
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

  CorruptedBoatState::CorruptorSet corruptors;
  corruptors.awa = CorruptedBoatState::Corruptor<Angle<double> >::offset(
      Angle<double>::degrees(9.8));
  corruptors.magHdg = CorruptedBoatState::Corruptor<Angle<double> >::offset(
      Angle<double>::degrees(-3.3));
  corruptors.aws = CorruptedBoatState::Corruptor<Velocity<double> >(1.09, Velocity<double>::knots(0.3));
  corruptors.watSpeed = CorruptedBoatState::Corruptor<Velocity<double> >(0.94, Velocity<double>::knots(-0.2));

  BoatSimulationSpecs specs(BoatCharacteristics(),
      dirs,        // <-- How the boat should be steered
      corruptors);

  return NavalSimulation(e, geoRef,
           simulationStartTime,
           wind,
           current,
           Array<BoatSimulationSpecs>::args(specs));
}


}
