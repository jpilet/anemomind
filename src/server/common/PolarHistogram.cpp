/*
 *  Created on: 2014-09-04
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PolarHistogram.h"
#include <cmath>
#include <server/common/math.h>
#include <server/common/logging.h>

namespace sail {

PolarHistogramMap::PolarHistogramMap(int binCount,
    double refBinIndex,
    Angle<double> refAngle) : _binCount(binCount) {
  CHECK(_binCount > 0);
  _binMapRadians = LineKM(refAngle.radians(), refAngle.radians() + 2.0*M_PI, // Angles
                                                                             // ... that map to ...
                          refBinIndex, refBinIndex + _binCount);             // bin indices
}

int PolarHistogramMap::angleToBin(Angle<double> value) const {
  return periodicIndex(int(floor(_binMapRadians(value.radians()))));
}

Angle<double> PolarHistogramMap::binIndexToAngle(double binIndex) const {
  return Angle<double>::radians(_binMapRadians.inv(binIndex));
}

Angle<double> PolarHistogramMap::toLeftBound(int binIndex) const {
  return binIndexToAngle(binIndex + 0.0);
}

Angle<double> PolarHistogramMap::toRightBound(int binIndex) const {
  return binIndexToAngle(binIndex + 1.0);
}

Angle<double> PolarHistogramMap::toCenter(int binIndex) const {
  return binIndexToAngle(binIndex + 0.5);
}

Arrayi PolarHistogramMap::countPerBin(Array<Angle<double> > angles) const {
  Arrayi counts = Arrayi::fill(_binCount, 0);
  for (auto angle : angles) {
    counts[angleToBin(angle)]++;
  }
  return counts;
}

Arrayi PolarHistogramMap::assignBins(Array<Angle<double> > values) const {
  return values.map<int>([&](Angle<double> x) {return angleToBin(x);});
}

int PolarHistogramMap::periodicIndex(int index) const {
  return positiveMod(index, _binCount);
}

Span<Angle<double> > PolarHistogramMap::binSpan(int binIndex) const {
  Angle<double> lower = toLeftBound(binIndex);

  // To ensure that it is always true that lower < upper.
  Angle<double> upper = lower + Angle<double>::radians(2.0*M_PI/_binCount);

  return Span<Angle<double> >(lower, upper);
}

} /* namespace mmm */
