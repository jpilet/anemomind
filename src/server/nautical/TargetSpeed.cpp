/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TargetSpeed.h"
#include <assert.h>
#include <algorithm>
#include <server/plot/extra.h>
#include <server/common/string.h>

namespace sail {

namespace {
  HorizontalMotion<double> estimateRawTrueWind(const Nav &nav) {
    // Apparent = True - BoatVel <=> True = Apparent + BoatVel
    // E.g. if we are sailing downwind, the apparent wind will be close to zero and
    // the true wind will be nearly the same as the boat velocity.
    // If we are sailing upwind, the true wind and boat vel will point in opposite directions and we will have a strong
    // apparent wind.
    return nav.apparentWind() + nav.gpsVelocity();
  }

  Angle<double> estimateRawTwa(const Nav &n) {
    return estimateRawTrueWind(n).angle() - n.gpsBearing();
  }

  Velocity<double> estimateRawTws(const Nav &n) {
    return estimateRawTrueWind(n).norm();
  }
}



Arrayd TargetSpeedData::makeDefaultQuantiles() {
  const int count = 5;

  // static, so that it remains in
  // memory after returning from this function.
  static double data[count] = {0.1, 0.25, 0.5, 0.75, 0.9};

  return Arrayd(count, data);
}

namespace {
  Arrayd extractQuantiles(
      Array<Velocity<double> > velocities,
      Arrayd quantiles) {

      if (velocities.empty()) {
        return Arrayd();
      }

      // E.g. with size = 3, qmap maps
      //  0.0 -> 0
      //  0.5 -> 1
      //  1.0 -> 2
      LineKM qmap(0.0, 1.0, 0, velocities.size() - 1);


      return quantiles.map<double>([&](double x) {
        return velocities[int(round(qmap(x)))].metersPerSecond();
      });
  }

  Arrayd toMPS(Array<Velocity<double> > speeds) {
    return speeds.map<double>([&](Velocity<double> x) {
          return x.metersPerSecond();
        });
  }

  bool validQuantiles(Arrayd Q) {
    for (auto q : Q) {
      if (q < 0 || 1 < q) {
        return false;
      }
    }
    return true;
  }
}

void TargetSpeedData::init(Array<Velocity<double> > windSpeeds,
                                 Array<Velocity<double> > vmg,

                                 HistogramMap map,
                                 Arrayd quantiles) {
  assert(!quantiles.empty());
  assert(validQuantiles(quantiles));
  Arrayd X = toMPS(windSpeeds);
  if (map.undefined()) {
    map = HistogramMap(10, X);
  }
  _hist = map;
  int count = windSpeeds.size();
  assert(vmg.size() == count);
  Array<Array<Velocity<double> > > groupedVmg =
      _hist.groupValuesByBin(X, vmg);
  for (auto &group : groupedVmg) {
    std::sort(group.begin(), group.end());
  }
  _medianValues = groupedVmg.map<Arrayd>(
      [&](const Array<Velocity<double> > &vmg) {
    return extractQuantiles(vmg, quantiles);
  });
  _quantiles = quantiles;
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

Array<Velocity<double> > calcVmg(Array<Nav> navs, bool isUpwind) {
  int sign = (isUpwind? 1 : -1);
  return navs.map<Velocity<double> >([&](const Nav &n) {
    double factor = sign*cos(estimateRawTwa(n));
    return n.gpsSpeed().scaled(factor);
  });
}

Array<Velocity<double> > calcUpwindVmg(Array<Nav> navs) {
  return calcVmg(navs, true);
}

Array<Velocity<double> > calcDownwindVmg(Array<Nav> navs) {
  return calcVmg(navs, false);
}

Array<Velocity<double> > estimateTws(Array<Nav> navs) {
  return navs.map<Velocity<double> >([&](const Nav &n) {return estimateRawTws(n);});
}


namespace {
  Arrayb markValidBins(Array<Arrayd> medianValues) {
    return medianValues.map<bool>([&](const Arrayd &x) {
      return x.hasData();
    });
  }

  Arrayd makeBinCenters(HistogramMap hist, Arrayb sel) {
    int count = countTrue(sel);
    Arrayd c(count);
    int counter = 0;
    for (int i = 0; i < sel.size(); i++) {
      if (sel[i]) {
        c[counter] = hist.toCenter(i);
        counter++;
      }
    }
    assert(counter == count);
    return c;
  }

}

void TargetSpeedData::plot() {
  Arrayb sel = markValidBins(_medianValues);
  Arrayd X = makeBinCenters(_hist, sel);
  Array<Arrayd> mvalues = _medianValues.slice(sel);
  GnuplotExtra plot;
  plot.set_style("lines");
  int qcount = _quantiles.size();
  for (int i = 0; i < qcount; i++) {
    Arrayd Y = mvalues.map<double>([&](const Array<double> &x) {
      return x[i];
    });
    assert(X.size() == Y.size());
    plot.plot_xy(X, Y, stringFormat("Quantile at %.3g", _quantiles[i]));
  }
  plot.set_xlabel("Wind Speed (m/s)");
  plot.set_ylabel("VMG (m/s)");
  plot.show();
}


} /* namespace sail */
