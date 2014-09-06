/*
 *  Created on: 2014-09-05
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BasicPolar.h"
#include <server/common/ArrayBuilder.h>
#include <server/plot/extra.h>
#include <server/common/string.h>

namespace sail {

PolarPoint::PolarPoint(Velocity<double> tws_,
    Angle<double> twa_,
    Velocity<double> boatSpeed_,
    int navIndex_): _twa(twa_),
    _tws(tws_), _boatSpeed(boatSpeed_),
    _navIndex(navIndex_) {}

PolarSlice::PolarSlice(TwaHist map,
  Array<PolarPoint> points) : _twaHist(map),
  _allPoints(points.size()) {

  // Fill the bins with points
  Array<ArrayBuilder<PolarPoint> > builders(map.binCount());
  for (auto p: points) {
    builders[map.toBin(p.twa())].add(p);
  }


  _pointsPerBin.create(map.binCount());
  int from = 0;
  for (int i = 0; i < _twaHist.binCount(); i++) {
    int to = from + builders[i].size();
    _pointsPerBin[i] = _allPoints.slice(from, to);
    builders[i].get().copyToSafe(_pointsPerBin[i]);
    from = to;
  }
  assert(from == _allPoints.size());

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

namespace {
  MDArray2d calcPolarXYPos(Array<PolarPoint> pts) {
    int count = pts.size();
    MDArray2d dst(count, 2);
    for (int i = 0; i < count; i++) {
      PolarPoint &pt = pts[i];
      dst(i, 0) = calcPolarX(true, pt.boatSpeed().knots(), pt.twa());
      dst(i, 1) = calcPolarY(true, pt.boatSpeed().knots(), pt.twa());
    }
    return dst;
  }
}

void PolarSlice::plot(double quantileFrac, GnuplotExtra *dst,
    const std::string &title) const{
  int n = _twaHist.binCount();
  Arrayd radii(n);
  for (int i = 0; i < n; i++) {
    radii[i] = lookUpBoatSpeedOr0(i, quantileFrac).knots();
  }
  dst->set_title(title);
  dst->set_xlabel("boatSpeed*sin(TWA) (knots)");
  dst->set_ylabel("boatSpeed*cos(TWA) (knots)");
  dst->set_style("points");
  dst->plot(calcPolarXYPos(_allPoints), "Samples");
  dst->set_style("lines");
  dst->plot(_twaHist.makePolarPlotData(radii, true));
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

void BasicPolar::plot(double quantileFrac) const {
  BasicPolar trimmed = trim();
  for (int i = 0; i < trimmed.twsHist().binCount(); i++) {
    std::string title = stringFormat("TWS ranging from %.3g knots to %.3g knots",
        trimmed.twsHist().toLeftBound(i).knots(),
        trimmed.twsHist().toRightBound(i).knots());
    GnuplotExtra dst;
    trimmed.slices()[i].plot(quantileFrac, &dst, title);
    dst.show();
  }
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
