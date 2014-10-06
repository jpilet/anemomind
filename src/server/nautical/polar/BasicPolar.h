/*
 *  Created on: 2014-09-05
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BASICPOLAR_H_
#define BASICPOLAR_H_

#include <server/common/Histogram.h>
#include <server/nautical/polar/PolarPoint.h>

namespace sail {

class GnuplotExtra;
class PolarSlice {
 public:
  typedef HistogramMap<Angle<double>, true> TwaHist;

  PolarSlice() {}
  PolarSlice(TwaHist map,
    Array<PolarPoint> points);

  int pointCount() const {
    return _allPoints.size();
  }

  TwaHist twaHist() {
    return _twaHist;
  }

  PolarPoint lookUpPolarPoint(int binIndex,
    double quantileFrac) const;

  Velocity<double> lookUpBoatSpeed(int binIndex,
      double quantileFrac) const {
    return lookUpPolarPoint(binIndex, quantileFrac).boatSpeed();
  }

  Velocity<double> lookUpBoatSpeedOr0(int binIndex,
      double quantileFrac) const {
    Velocity<double> v = lookUpBoatSpeed(binIndex, quantileFrac);
    if (v.isNaN()) {
      return Velocity<double>::knots(0);
    }
    return v;
  }

  PolarPoint lookUpPolarPoint(Angle<double> twa, double qfrac) const {
    return lookUpPolarPoint(_twaHist.toBin(twa), qfrac);
  }

  Velocity<double> lookUpBoatSpeed(Angle<double> twa, double qfrac) const {
    return lookUpBoatSpeed(_twaHist.toBin(twa), qfrac);
  }

  Velocity<double> lookUpBoatSpeedOr0(Angle<double> twa, double qfrac) const {
    return lookUpBoatSpeedOr0(_twaHist.toBin(twa), qfrac);
  }

  bool empty() const {
    return pointCount() <= 0;
  }

  void plot(double quantileFrac, GnuplotExtra *dst, const std::string &title = "(no title)") const;
 private:
  TwaHist _twaHist;
  Array<Array<PolarPoint> > _pointsPerBin;
  Array<PolarPoint> _allPoints;
};

class BasicPolar {
 public:
  typedef HistogramMap<Velocity<double>, false> TwsHist;

  BasicPolar() {}

  BasicPolar(TwsHist twsHist, Array<PolarSlice> slices) :
    _twsHist(twsHist), _slices(slices) {
    assert(twsHist.binCount() == slices.size());
  }

  BasicPolar(TwsHist twsHist_,
      PolarSlice::TwaHist twaHist_,
      Array<PolarPoint> points);

  BasicPolar slice(int fromBinIndex, int toBinIndex) const {
    return BasicPolar(_twsHist.slice(fromBinIndex, toBinIndex),
        _slices.slice(fromBinIndex, toBinIndex));
  }

  BasicPolar trim() const;

  const Array<PolarSlice> &slices() const {
    return _slices;
  }

  const TwsHist &twsHist() const {
    return _twsHist;
  }

  int pointCount() const;

  bool empty() const {
    // We should be clear what we mean by this. Does it mean _slices.empty() or pointCount() == 0?
    return _slices.empty();
  }

  void plot(double quantileFrac) const;
 private:
  TwsHist _twsHist;
  Array<PolarSlice> _slices;
};

}

#endif /* BASICPOLAR_H_ */
