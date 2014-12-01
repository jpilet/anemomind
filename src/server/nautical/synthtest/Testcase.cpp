/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Testcase.h"

namespace sail {

Testcase::FlowFun Testcase::constantFlowFun(HorizontalMotion<double> m) {
  return [=](const ProjectedPosition &pos, Duration<double> dur) {
    return m;
  };
}

Testcase::BoatSpecs::BoatSpecs(BoatCharacteristics ch, Array<Dir> dirs,
  CorruptedBoatState::CorruptorSet corruptors,
  Nav::Id boatId, Duration<double> samplingPeriod, int stepsPerSample) :
    _ch(ch),
    _dirs(dirs),
    _corruptors(corruptors),
    _indexer(ProportionateIndexer(dirs.size(),
        [=](int index) {return dirs[index].dur.seconds();})),
        _boatId(boatId), _samplingPeriod(samplingPeriod),
        _stepsPerSample(stepsPerSample) {}


Angle<double> Testcase::BoatSpecs::twa(Duration<double> dur) const {
  auto result = _indexer.get(dur.seconds());
  return _dirs[result.index].interpolate(result.localX);
}

Testcase::Testcase(std::default_random_engine &e,
         GeographicReference geoRef,
         TimeStamp timeOffset,
         FlowFun wind,
         FlowFun current,
         Array<BoatSpecs> specs) :
         _geoRef(geoRef),
         _timeOffset(timeOffset),
         _wind(wind),
         _current(current), _boatData(specs.size()) {
  for (int i = 0; i < specs.size(); i++) {
    //BoatSim sim()
  }
}


} /* namespace mmm */
