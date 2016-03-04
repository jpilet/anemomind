/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TargetSpeed.h"
#include <algorithm>
#include <cassert>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <device/Arduino/libraries/ChunkFile/ChunkFile.h>
#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>
#include <server/common/logging.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/string.h>
#include <server/common/LineKM.h>
#include <server/common/Functional.h>

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
    return apparentWind(nav) + nav.gpsMotion();
  }

  Angle<double> estimateRawTwa(const Nav &n) {
    LOG(FATAL) << "The results from this function may not be correct";
    return estimateRawTrueWind(n).angle() - n.gpsBearing() - fromAngle;
  }

  Velocity<double> estimateRawTws(const Nav &n) {
    LOG(FATAL) << "The results from this function may not be correct";
    return estimateRawTrueWind(n).norm();
  }

  int lookUp(Array<Velocity<double> > bounds, Velocity<double> tws) {
    if (tws < bounds.first() || bounds.last() <= tws || tws.isNaN()) {
      return -1;
    }
    Array<Velocity<double> >::Iterator i = std::upper_bound(bounds.begin(), bounds.end(), tws);
    int index = i - bounds.begin() - 1;
    assert(0 <= index);
    assert(index < bounds.size()-1);
    return index;
  }

  Arrayi lookUp(Array<Velocity<double> > bounds, Array<Velocity<double> > tws) {
    return toArray(map(tws, [&](Velocity<double> x) {return lookUp(bounds, x);}));
  }

  Array<Array<Velocity<double> > > groupVmg(int binCount, Arrayi bins, Array<Velocity<double> > vmg) {
    Array<ArrayBuilder<Velocity<double> > > builders(binCount);
    int count = vmg.size();
    for (int i = 0; i < count; i++) {
      int bin = bins[i];
      if (bin != -1) {
        builders[bin].add(vmg[i]);
      }
    }
    Array<Array<Velocity<double> > > groups = toArray(
        map(builders, [=](ArrayBuilder<Velocity<double> > x) {return x.get();}));
    for (int i = 0; i < binCount; i++) {
      std::sort(groups[i].begin(), groups[i].end());
    }
    return groups;
  }

  void outputMedianValues(Array<Velocity<double> > data, Arrayd quantiles, int index, Array<Array<Velocity<double> > > out) {
    int qCount = quantiles.size();
    LineKM map(0, 1.0, 0, data.size() - 1);
    for (int i = 0; i < qCount; i++) {
      int i2 = int(round(map(quantiles[i])));
      if (0 <= i2 && i2 < data.size()) {
        out[i][index] = data[i2];
      } else {
        out[i][index] = Velocity<double>::knots(NAN);
      }
    }
  }
}

int lookUpForUnitTest(Array<Velocity<double> > bounds, Velocity<double> tws) {
  return lookUp(bounds, tws);
}









TargetSpeed::TargetSpeed(bool isUpwind_, Array<Velocity<double> > tws,
    Array<Velocity<double> > vmg,
    Array<Velocity<double> > bounds, Arrayd quantiles_) {
  isUpwind = isUpwind_;
  quantiles = quantiles_;


  Arrayi bins = lookUp(bounds, tws);
  int binCount = bounds.size() - 1;
  Array<Array<Velocity<double > > > groups = groupVmg(binCount, bins, vmg);

  int qCount = quantiles.size();
  binCenters = Array<Velocity<double> >(binCount);
  medianValues = Array<Array<Velocity<double> > >::fill(qCount, [=](int i) {return Array<Velocity<double> >(binCount);});
  for (int i = 0; i < binCount; i++) {
    binCenters[i] = (bounds[i] + bounds[i + 1]).scaled(0.5);
    outputMedianValues(groups[i], quantiles, i, medianValues);
  }
}

void TargetSpeed::plot() {
  GnuplotExtra plot;
  plot.set_style("lines");
  plot.set_xlabel("TWS (knots)");
  plot.set_ylabel("VMG (knots)");
  plot.set_title((isUpwind? "Upwind" : "Downwind"));
  auto mapper = [](Velocity<double> x) {return x.knots();};
  for (int i = 0; i < medianValues.size(); i++) {
    plot.plot_xy(map(binCenters, mapper), map(medianValues[i], mapper), stringFormat("Quantile %.3g", quantiles[i]));
  }
  plot.show();
}

Array<Velocity<double> > makeBoundsFromBinCenters(int binCount,
    Velocity<double> minBinCenter,
    Velocity<double> maxBinCenter) {
    int boundCount = binCount + 1;
    LineKM boundMap(0.0 + 0.5, boundCount-1 - 0.5, minBinCenter.knots(), maxBinCenter.knots());
    return Array<Velocity<double> >::fill(boundCount, [=](int i) {
      return Velocity<double>::knots(boundMap(i));
    });
}

Arrayd TargetSpeed::makeDefaultQuantiles() {
  const int count = 5;

  // static, so that it remains in
  // memory after returning from this function.
  static double data[count] = {0.1, 0.25, 0.5, 0.75, 0.9};

  return Arrayd(count, data);
}

Array<Velocity<double> > calcVmg(NavDataset navs, bool isUpwind) {
  int sign = (isUpwind? 1 : -1);
  return toArray(map(NavCompat::Range(navs), [&](const Nav &n) {
    double factor = sign*cos(estimateRawTwa(n));
    return n.gpsSpeed().scaled(factor);
  }));
}

Array<Velocity<double> > calcExternalVmg(NavDataset navs, bool isUpwind) {
  int sign = isUpwind? 1 : -1;
  return toArray(map(NavCompat::Range(navs), [&](const Nav &n) {
    double factor = sign*cos(n.externalTwa());
    return n.gpsSpeed().scaled(factor);
  }));
}


Array<Velocity<double> > calcUpwindVmg(NavDataset navs) {
  return calcVmg(navs, true);
}

Array<Velocity<double> > calcDownwindVmg(NavDataset navs) {
  return calcVmg(navs, false);
}

Array<Velocity<double> > estimateTws(NavDataset navs) {
  return toArray(map(NavCompat::Range(navs), [&](const Nav &n) {return estimateRawTws(n);}));
}

Array<Velocity<double> > estimateExternalTws(NavDataset navs) {
  return toArray(map(NavCompat::Range(navs), [&](const Nav &n) {return n.externalTws();}));
}


namespace {

  double minus1IfNan(double x) {
    if (std::isnan(x)) {
      return -1;
    }
    return x;
  }
}

void saveTargetSpeedTableChunk(
    ostream *stream,
    const TargetSpeed& upwind,
    const TargetSpeed& downwind) {
  TargetSpeedTable table;
  CHECK(table.NUM_ENTRIES == upwind.binCenters.size());
  CHECK(table.NUM_ENTRIES == downwind.binCenters.size());
  for (int i = 0; i < TargetSpeedTable::NUM_ENTRIES; ++i) {
    table._upwind[i] = FP8_8(minus1IfNan(upwind.medianValues.last()[i].knots()));
    table._downwind[i] = FP8_8(minus1IfNan(downwind.medianValues.last()[i].knots()));
  }
  writeChunk(*stream, &table);
}


} /* namespace sail */
