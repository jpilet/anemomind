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

Testcase::BoatSpecs::BoatSpecs(BoatCharacteristics ch, Array<Dir> dirs) : _ch(ch), _dirs(dirs) {
}


Angle<double> Testcase::BoatSpecs::twa(Duration<double> dur) const {
  auto result = _indexer.get(dur.seconds());
  return _dirs[result.index].interpolate(result.localX);
}


} /* namespace mmm */
