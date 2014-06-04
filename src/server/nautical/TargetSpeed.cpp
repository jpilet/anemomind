/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TargetSpeed.h"
#include <assert.h>
#include <algorithm>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <device/Arduino/libraries/ChunkFile/ChunkFile.h>
#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>
#include <server/common/logging.h>

namespace sail {

namespace {
  // This angle is added/subtracted in some places due to the fact
  // that sailors usually give the angle to the vector _from_ which
  // the wind is blowing and not the angle of the vector _in_ which
  // the wind is blowing.
  const Angle<double> fromAngle = Angle<double>::radians(M_PI);

  HorizontalMotion<double> apparentWind(const Nav &nav) {
  /* Important note: awa() is the angle w.r.t. the course of the boat!
   * So awa() = 0 always means the boat is in irons.
   * Therefore, to get the apparent wind motion w.r.t. earth, we also
   * have to add the course of the boat to that. */
    LOG(FATAL) << "The results from this function may not be correct";
    return HorizontalMotion<double>::polar(nav.aws(), nav.awa() + nav.gpsBearing() + fromAngle);
  }

  HorizontalMotion<double> estimateRawTrueWind(const Nav &nav) {
    // Apparent = True - BoatVel <=> True = Apparent + BoatVel
    // E.g. if we are sailing downwind, the apparent wind will be close to zero and
    // the true wind will be nearly the same as the boat velocity.
    // If we are sailing upwind, the true wind and boat vel will point in opposite directions and we will have a strong
    // apparent wind.
    LOG(FATAL) << "The results from this function may not be correct";
    return apparentWind(nav) + nav.gpsVelocity();
  }

  Angle<double> estimateRawTwa(const Nav &n) {
    LOG(FATAL) << "The results from this function may not be correct";
    return estimateRawTrueWind(n).angle() - n.gpsBearing() - fromAngle;
  }

  Velocity<double> estimateRawTws(const Nav &n) {
    LOG(FATAL) << "The results from this function may not be correct";
    return estimateRawTrueWind(n).norm();
  }

  double max(const Arrayd &array, double additionalValue) {
    return array.reduce<double>(additionalValue, [] (double a, double b) { return std::max(a, b); });
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
      Arrayd quantiles, std::function<double(Velocity<double>)> unwrapper) {

      if (velocities.empty()) {
        return Arrayd();
      }

      // E.g. with size = 3, qmap maps
      //  0.0 -> 0
      //  0.5 -> 1
      //  1.0 -> 2
      LineKM qmap(0.0, 1.0, 0, velocities.size() - 1);


      return quantiles.map<double>([&](double x) {
        return unwrapper(velocities[int(round(qmap(x)))]);
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
  Arrayd X = windSpeeds.map<double>(makeUnwrapper());
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
    return extractQuantiles(vmg, quantiles, makeUnwrapper());
  });
  _quantiles = quantiles;
}


TargetSpeedData::TargetSpeedData(Array<Velocity<double> > windSpeeds,
    Array<Velocity<double> > vmg,
    int binCount, Velocity<double> minTws, Velocity<double> maxTws,
    Arrayd quantiles) {
  init(windSpeeds, vmg, HistogramMap(binCount,
      unwrap(minTws), unwrap(maxTws)), quantiles);
}

Array<Velocity<double> > calcVmg(Array<Nav> navs, bool isUpwind) {
  int sign = (isUpwind? 1 : -1);
  return navs.map<Velocity<double> >([&](const Nav &n) {
    double factor = sign*cos(estimateRawTwa(n));
    return n.gpsSpeed().scaled(factor);
  });
}

Array<Velocity<double> > calcExternalVmg(Array<Nav> navs, bool isUpwind) {
  int sign = (isUpwind? 1 : -1);
  return navs.map<Velocity<double> >([&](const Nav &n) {
    double factor = sign*cos(n.externalTwa());
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

Array<Velocity<double> > estimateExternalTws(Array<Nav> navs) {
  return navs.map<Velocity<double> >([&](const Nav &n) {return n.externalTws();});
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
  plot.set_xlabel("Wind Speed (knots)");
  plot.set_ylabel("VMG (knots)");
  plot.show();
}

Arrayd TargetSpeedData::targetVmgForWindSpeed(Velocity<double> windSpeed) const {
  // This is "nearest" sampling. TODO: linear interpolation.
  int bin = _hist.toBin(windSpeed.knots());
  return _medianValues[bin];
}

void saveTargetSpeedTableChunk(
    ostream *stream,
    const TargetSpeedData& upwind,
    const TargetSpeedData& downwind) {
  TargetSpeedTable table;
  for (int knots = 0; knots < TargetSpeedTable::NUM_ENTRIES; ++knots) {
    Velocity<double> binCenter = Velocity<double>::knots(double(knots) + .5);
    table._upwind[knots] = FP8_8(max(upwind.targetVmgForWindSpeed(binCenter), -1.0));
    table._downwind[knots] = FP8_8(max(downwind.targetVmgForWindSpeed(binCenter), -1.0));
  }
  writeChunk(*stream, &table);
}

} /* namespace sail */
