/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BasicPolar.h"
#include <server/common/ArrayBuilder.h>

namespace sail {

PolarPoint::PolarPoint(Angle<double> twa_,
    Velocity<double> tws_,
    Velocity<double> boatSpeed_,
    int navIndex_): _twa(twa_),
    _tws(tws_), _boatSpeed(boatSpeed_),
    _navIndex(navIndex_) {}

Polar2d::Polar2d(PolarHistMap map,
  Array<PolarPoint> points) : _histmap(map) {
  Array<ArrayBuilder<PolarPoint> > builders(map.binCount());
  for (auto p: points) {
    builders[map.toBin(p.twa())].add(p);
  }
  _pointsPerBin = builders.map<Array<PolarPoint> >([&](ArrayBuilder<PolarPoint> b) {
    return b.get();
  });
  for (auto b: _pointsPerBin) {
    std::sort(b.begin(), b.end());
  }
}




BasicPolar::BasicPolar() {
  // TODO Auto-generated constructor stub

}

BasicPolar::~BasicPolar() {
  // TODO Auto-generated destructor stub
}

} /* namespace mmm */
