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
#include <server/common/ArrayBuilder.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>

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

int lookUp(Array<Velocity<double> > bounds, Velocity<double> tws) {
  int count = bounds.size();
  if (bounds.last() <= tws) {
    return -1;
  }
  for (int i = count-1; i >= 0; i--) {
    if (bounds[i] <= tws) {
      return i;
    }
  }
  return -1;
}

Arrayi lookUp(Array<Velocity<double> > bounds, Array<Velocity<double> > tws) {
  int count = tws.size();
  Arrayi bins(count);
  for (int i = 0; i < count; i++) {
    bins[i] = lookUp(bounds, tws[i]);
  }
  return bins;
}

Array<Array<Velocity<double> > > groupVmg(bool isUpwind, int binCount, Arrayi bins, Array<Velocity<double> > vmg) {
  Array<ArrayBuilder<Velocity<double> > > builders(binCount);
  int count = vmg.size();
  int sign = (isUpwind? 1 : -1);
  for (int i = 0; i < count; i++) {
    int bin = bins[i];
    if (bin != -1) {
      builders[bin].add(vmg[i].scaled(sign));
    }
  }
  Array<Array<Velocity<double> > > groups = builders.map<Array<Velocity<double> > >([=](ArrayBuilder<Velocity<double> > x) {return x.get();});
  for (int i = 0; i < binCount; i++) {
    std::sort(groups[i].begin(), groups[i].end());
  }
  return groups;
}

void outputMedianValues(Array<Velocity<double> > data, Arrayd quantiles, int index, Array<Array<Velocity<double> > > out) {
  int qCount = quantiles.size();
  LineKM map(0, 1.0, 0, data.size() - 1);
  int count = data.size();
  for (int i = 0; i < qCount; i++) {
    int i2 = int(round(map(quantiles[i])));
    if (0 <= i2 && i2 < data.size()) {
      out[i][index] = data[i2];
    } else {
      out[i][index] = Velocity<double>::knots(-1);
    }
  }
}

RefImplTgtSpeed::RefImplTgtSpeed(bool isUpwind_, Array<Velocity<double> > tws,
    Array<Velocity<double> > vmg,
    Array<Velocity<double> > bounds, Arrayd quantiles_) {
  isUpwind = isUpwind_;
  quantiles = quantiles_;

  std::cout << EXPR_AND_VAL_AS_STRING(bounds
      .map<double>([=](Velocity<double> x) {return x.knots();})) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(   tws.sliceTo(30)
      .map<double>([=](Velocity<double> x) {return x.knots();})) << std::endl;

  Arrayi bins = lookUp(bounds, tws);
  int binCount = bounds.size() - 1;
  Array<Array<Velocity<double > > > groups = groupVmg(isUpwind, binCount, bins, vmg);
  std::cout << EXPR_AND_VAL_AS_STRING(groups.map<int>([=](Array<Velocity<double> > x) {return x.size();})) << std::endl;

  int qCount = quantiles.size();
  binCenters = Array<Velocity<double> >(binCount);
  medianValues = Array<Array<Velocity<double> > >::fill(qCount, [=](int i) {return Array<Velocity<double> >(binCount);});
  for (int i = 0; i < binCount; i++) {
    binCenters[i] = (bounds[i] + bounds[i + 1]).scaled(0.5);
    outputMedianValues(groups[i], quantiles, i, medianValues);
  }
}

void RefImplTgtSpeed::plot() {
  GnuplotExtra plot;
  plot.set_style("lines");
  plot.set_xlabel("TWS (knots)");
  plot.set_ylabel("VMG (knots)");
  plot.set_title((isUpwind? "Upwind" : "Downwind"));
  auto mapper = [](Velocity<double> x) {return x.knots();};
  for (int i = 0; i < medianValues.size(); i++) {
    plot.plot_xy(binCenters.map<double>(mapper), medianValues[i].map<double>(mapper), stringFormat("Quantile %.3g", quantiles[i]));
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
























Arrayd RefImplTgtSpeed::makeDefaultQuantiles() {
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

class TargetSpeedData {
 public:
  TargetSpeedData() {}
  TargetSpeedData(Array<Velocity<double> > windSpeeds,
      Array<Velocity<double> > vmg,
      int binCount, Velocity<double> minTws, Velocity<double> maxTws,
      Arrayd quantiles = RefImplTgtSpeed::makeDefaultQuantiles());
  void plot(std::string title = "Untitled target speed");

  Arrayd targetVmgForWindSpeed(Velocity<double> windSpeed) const;
 private:
  void init(Array<Velocity<double> > windSpeeds,
      Array<Velocity<double> > vmg,
      HistogramMap map,
      Arrayd quantiles);

  Velocity<double> wrap(double x) {return Velocity<double>::knots(x);}
  double unwrap(Velocity<double> x) {return x.knots();}
  std::function<double(Velocity<double>)> makeUnwrapper() {return [=](Velocity<double> x) {return unwrap(x);};}

  // All velocities are internally stored as [knots]
  // Such a convention is reasonable, since HistogramMap only
  // works with doubles.
  Arrayd _quantiles;
  HistogramMap _hist;
  Array<Arrayd> _medianValues;
};

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

Array<Velocity<double> > calcExternalUnsigned(Array<Nav> navs) {
  return calcExternalVmg(navs, true);
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

void TargetSpeedData::plot(std::string title) {
  Arrayb sel = markValidBins(_medianValues);
  Arrayd X = makeBinCenters(_hist, sel);
  Array<Arrayd> mvalues = _medianValues.slice(sel);
  GnuplotExtra plot;
  plot.set_grid();
  plot.set_style("lines");
  plot.set_xlabel("Wind Speed (knots)");
  plot.set_ylabel("VMG (knots)");
  plot.set_title(title);

  for (int i = 0; i < _quantiles.size(); i++) {
    Arrayd Y = mvalues.map<double>([&](const Array<double> &x) {
      return x[i];
    });
    assert(X.size() == Y.size());
    plot.plot_xy(X, Y, stringFormat("Quantile at %.3g", _quantiles[i]));
  }
  plot.show();
}

Arrayd TargetSpeedData::targetVmgForWindSpeed(Velocity<double> windSpeed) const {
  // This is "nearest" sampling. TODO: linear interpolation.
  int bin = _hist.toBin(windSpeed.knots());
  if (bin == -1) { // If it falls outside the bin
    return Arrayd();
  }
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

void saveTargetSpeedTableChunk(
    ostream *stream,
    const RefImplTgtSpeed& upwind,
    const RefImplTgtSpeed& downwind) {
  TargetSpeedTable table;
  CHECK(table.NUM_ENTRIES == upwind.binCenters.size());
  CHECK(table.NUM_ENTRIES == downwind.binCenters.size());
  CHECK(  upwind.quantileCount() == 1);
  CHECK(downwind.quantileCount() == 1);
  for (int i = 0; i < TargetSpeedTable::NUM_ENTRIES; ++i) {
    table._upwind[i] = FP8_8(max(upwind.medianValues[0][i].knots(), -1.0));
    table._downwind[i] = FP8_8(max(downwind.medianValues[0][i].knots(), -1.0));
  }
  writeChunk(*stream, &table);
}


} /* namespace sail */
