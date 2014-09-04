/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARHISTOGRAM_H_
#define POLARHISTOGRAM_H_

#include <server/common/LineKM.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/Array.h>
#include <server/common/Span.h>

namespace sail {

class PolarHistogramMap {
 public:
  PolarHistogramMap() : _binCount(0) {}

  PolarHistogramMap(int binCount,
      double refBinIndex = 0,
      Angle<double> refAngle = Angle<double>::radians(0));
  int toBin(Angle<double> value) const;
  Angle<double> binIndexToAngle(double binIndex) const;
  Angle<double> toLeftBound(int binIndex) const;
  Angle<double> toRightBound(int binIndex) const;
  Angle<double> toCenter(int binIndex) const;
  Arrayi countPerBin(Array<Angle<double> > angles) const;
  Arrayi assignBins(Array<Angle<double> > values) const;
  int periodicIndex(int index) const;
  Span<Angle<double> > binSpan(int binIndex) const;

  int binCount() const {return _binCount;}
  bool defined() const {return _binCount > 0;}
  bool undefined() const {return !defined();}
 private:
  int _binCount;
  LineKM _binMapRadians;
};

} /* namespace mmm */

#endif /* POLARHISTOGRAM_H_ */
