/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Testcase.h"

namespace sail {

BoatSimulation::FlowFun BoatSimulation::constantFlowFun(HorizontalMotion<double> m) {
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
        [=](int index) {return dirs[index].dur.seconds();})),
        _boatId(boatId), _samplingPeriod(samplingPeriod),
        _stepsPerSample(stepsPerSample) {}


Angle<double> BoatSimulationSpecs::twa(Duration<double> dur) const {
  auto result = _indexer.get(dur.seconds());
  return _dirs[result.index].interpolate(result.localX);
}

BoatSimulation::BoatSimulation(std::default_random_engine &e,
         GeographicReference geoRef,
         TimeStamp timeOffset,
         FlowFun wind,
         FlowFun current,
         Array<BoatSimulationSpecs> specs) :
         _geoRef(geoRef),
         _timeOffset(timeOffset),
         _wind(wind),
         _current(current), _boatData(specs.size()) {
  for (int i = 0; i < specs.size(); i++) {
    auto sp = specs[i];
    BoatSim sim(wind, current, sp.characteristics(), [=](Duration<double> dur) {return sp.twa(dur);});
    Array<BoatSim::FullState> states = sim.simulate(sp.duration(), sp.samplingPeriod(), sp.stepsPerSample());
    _boatData[i] = makeBoatData(sp, states, e);
  }
}

BoatSimulation::BoatData BoatSimulation::makeBoatData(BoatSimulationSpecs &specs,
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


}
