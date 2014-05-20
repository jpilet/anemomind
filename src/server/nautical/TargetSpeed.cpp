/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TargetSpeed.h"
#include <assert.h>
#include <algorithm>

namespace sail {

Arrayd TargetSpeedData::makeDefaultQuantiles() {
  const int count = 5;

  // static, so that it remains in
  // memory after returning from this function.
  static double data[count] = {0.1, 0.25, 0.5, 0.75, 0.9};

  return Arrayd(count, data);
}

namespace {
  Array<Velocity<double> > extractQuantiles(
      Array<Velocity<double> > velocities,
      Arrayd quantiles) {

      if (velocities.empty()) {
        return Array<Velocity<double> >();
      }

      // E.g. with size = 3, qmap maps
      //  0.0 -> 0
      //  0.5 -> 1
      //  1.0 -> 2
      LineKM qmap(0.0, 1.0, 0, velocities.size() - 1);


      return quantiles.map<Velocity<double> >([&](double x) {
        return velocities[int(round(qmap(x)))];
      });
  }

  Arrayd toMPS(Array<Velocity<double> > speeds) {
    return speeds.map<double>([&](Velocity<double> x) {
          return x.metersPerSecond();
        });
  }
}

void TargetSpeedData::init(Array<Velocity<double> > windSpeeds,
                                 Array<Velocity<double> > vmg,

                                 HistogramMap map,
                                 Arrayd quantiles) {
  Arrayd X = toMPS(windSpeeds);
  if (map.undefined()) {
    map = HistogramMap(10, X);
  }
  _hist = map;
  assert(!quantiles.empty());
  int count = windSpeeds.size();
  assert(vmg.size() == count);
  Array<Array<Velocity<double> > > groupedVmg =
      _hist.groupValuesByBin(X, windSpeeds);
  for (auto &group : groupedVmg) {
    std::sort(group.begin(), group.end());
  }
  _medianValues = groupedVmg.map<Array<Velocity<double> > >(
      [&](const Array<Velocity<double> > &vmg) {
    return extractQuantiles(vmg, quantiles);
  });
}

TargetSpeedData::TargetSpeedData(Array<Velocity<double> > windSpeeds,
    Array<Velocity<double> > vmg,
    HistogramMap map,
    Arrayd quantiles) {
    init(windSpeeds, vmg, map, quantiles);
}

TargetSpeedData::TargetSpeedData(Array<Velocity<double> > windSpeeds,
    Array<Velocity<double> > vmg,
    int binCount,
    Arrayd quantiles) {
  init(windSpeeds, vmg, HistogramMap(binCount,
      toMPS(windSpeeds)), quantiles);
}

Array<Velocity<double> > getVmg(Array<Nav> navs, bool isUpwind) {
  int sign = (isUpwind? 1 : -1);
  return navs.map<Velocity<double> >([&](const Nav &n) {
    double factor = sign*cos(n.estimateRawTwa());
    return n.gpsSpeed().scaled(factor);
  });
}

Array<Velocity<double> > getUpwindVmg(Array<Nav> navs) {
  return getVmg(navs, true);
}

Array<Velocity<double> > getDownwindVmg(Array<Nav> navs) {
  return getVmg(navs, false);
}



} /* namespace sail */
