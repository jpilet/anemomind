/*
 *  Created on: 2014-09-05
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

PolarSlice::PolarSlice(TwaHist map,
  Array<PolarPoint> points) : _twaHist(map),
  _pointCount(points.size()) {

  // Fill the bins with points
  Array<ArrayBuilder<PolarPoint> > builders(map.binCount());
  for (auto p: points) {
    builders[map.toBin(p.twa())].add(p);
  }
  _pointsPerBin = builders.map<Array<PolarPoint> >([&](ArrayBuilder<PolarPoint> b) {
    return b.get();
  });

  // Sort every bin to make it easy to extract quantiles
  for (auto b: _pointsPerBin) {
    std::sort(b.begin(), b.end());
  }
}

PolarPoint PolarSlice::lookUpPolarPoint(int binIndex,
  double quantileFrac) const {
  Array<PolarPoint> pts = _pointsPerBin[binIndex];
  if (pts.empty()) {
    return PolarPoint();
  } else {
    assert(0 <= quantileFrac);
    assert(quantileFrac <= 1.0);
    int index = int(round(quantileFrac*pts.size()));
    if (index == pts.size()) {
      return pts.last();
    } else {
      return pts[index];
    }
  }
}


BasicPolar::BasicPolar(TwsHist twsHist_,
    PolarSlice::TwaHist twaHist_,
    Array<PolarPoint> points) : _twsHist(twsHist_) {
  Array<ArrayBuilder<PolarPoint> > builders(twsHist_.binCount());
  for (auto p: points) {
    builders[twsHist_.toBin(p.tws())].add(p);
  }
  _slices = builders.map<PolarSlice>([&](ArrayBuilder<PolarPoint> b) {
    return PolarSlice(twaHist_, b.get());
  });
}

int BasicPolar::pointCount() const {
  int count = 0;
  for (auto s: _slices) {
    count += s.pointCount();
  }
  return count;
}

namespace {
  int getLeft(const Array<PolarSlice> &slices) {
    for (int i = 0; i < slices.size(); i++) {
      if (!slices[i].empty()) {
        return i;
      }
    }
    return slices.size();
  }

  int getRight(const Array<PolarSlice> &slices) {
    for (int i = slices.size(); i >= 1; i--) {
      if (!slices[i-1].empty()) {
        return i;
      }
    }
    return 0;
  }
}


BasicPolar BasicPolar::trim() const {
  int left = getLeft(_slices);
  int right = getRight(_slices);
  if (left >= right) {
    return BasicPolar(TwsHist(), Array<PolarSlice>());
  } else {
    return slice(left, right);
  }
}




} /* namespace mmm */
