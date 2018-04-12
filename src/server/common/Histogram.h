/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <server/common/Array.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/MDArray.h>
#include <server/common/Span.h>
#include <server/common/math.h>
#include <server/transducers/Transducer.h>

namespace sail {

template<typename Source>
struct CastToDoubleFunctor {
  double operator()(Source x) const { return static_cast<double>(x); }
};

namespace HistogramInternal {
  template <typename T>
  Array<Array<T> > allocateGroups(Arrayi cpb) {
    return transduce(cpb, trMap([&](int i) {return Array<T>(i);}), IntoArray<Array<T>>());
  }

  void drawBin(double left, double right, double height, MDArray2d dst);
  void drawPolarBin(double leftRadians, double rightRadians, double radius,
      MDArray2d dst, int segmentsPerBin, bool nautical);
}

template <typename T, bool periodic>
class HistogramMap {
 public:
  typedef HistogramMap<T, periodic> ThisType;

  HistogramMap() : _binCount(0) {}

  HistogramMap(int count, T leftBd, T rightBd, double indexShift = 0) {
    init(count, leftBd, rightBd, indexShift);
  }

  HistogramMap(int count, Array<T> values) {
    assert(!periodic);
    Span<T> span(values);
    const double marg = 1.0e-9;
    init(count, span.minv() - marg, span.maxv() + marg);
  }

  int binCount() const {return _binCount;}

  int toBin(T value) const {
    int index = int(floor((value - _m)/_k));
    if (periodic) {
      return periodicIndex(index);
    } else if (validIndex(index)) {
      return index;
    } else {
      return -1;
    }
  }

  T binToValue(double binIndex) const {
    return binIndex*_k + _m;
  }

  T toLeftBound(int binIndex) const {
    return binToValue(binIndex);
  }

  T toRightBound(int binIndex) const {
    return binToValue(binIndex + 1.0);
  }

  ThisType slice(int fromBinIndex, int toBinIndex) const {
    return ThisType(toBinIndex - fromBinIndex, // Number of bins in the slice
        toLeftBound(fromBinIndex),             // Left bound of the first cell
        toRightBound(toBinIndex-1));           // Right bound of the last cell (that has index toBinIndex-1)
  }

  T toCenter(int binIndex) const {
    return binToValue(binIndex + 0.5);
  }

  Span<T> binSpan(int binIndex) const {
    T offs = toLeftBound(binIndex);
    return Span<T>(offs, offs + _k);
  }

  Arrayi countPerBin(Array<T> values) const {
    Arrayi hist = Arrayi::fill(_binCount, 0);
    for (auto value: values) {
      int index = toBin(value);
      if (validIndex(index)) {
        hist[index]++;
      }
    }
    return hist;
  }

  Arrayi assignBins(Array<T> values) const {
    //return values.map<int>([&](T x) {return toBin(x);});
    int count = values.size();
    Arrayi dst(count);
    for (int i = 0; i < count; i++) {
      dst[i] = toBin(values[i]);
    }
    return dst;
  }

  bool defined() const {return _binCount > 0;}
  bool undefined() const {return !defined();}

  // Returns true if the index refers to a valid element in
  // the target array. Therefore, it applies to both
  // periodic and non-periodic histograms.
  bool validIndex(int index) const {
    return 0 <= index && index < _binCount;
  }


  int periodicIndex(int i) const {
    return positiveMod(i, _binCount);
  }


  template<class Cast = CastToDoubleFunctor<T>>
  MDArray2d makePlotData(Arrayi counts, Cast castToDouble = Cast()) const {
    assert(_binCount == counts.size());
    MDArray2d plotData(3*_binCount + 1, 2);
    for (int i = 0; i < _binCount; i++) {
      double left = castToDouble(toLeftBound(i));
      double right = castToDouble(toRightBound(i));
      HistogramInternal::drawBin(left, right, counts[i], plotData.sliceRowBlock(i, 3));
    }
    int lastRow = plotData.rows()-1;
    plotData(lastRow, 0) = castToDouble(toRightBound(_binCount-1));
    plotData(lastRow, 1) = 0;
    return plotData;
  }

  // The radii could for instance be the count in every bin
  // or the 80% percentile value in a bin.
  //
  // 'nautical = true' means that the x-position is radius*sin(angle) and
  // the y-position is radius*cos(angle). This is suitable for making a
  // polar plot for target speed.
  //
  // 'nautical = false' means that the x-position is radius*cos(angle) and
  // the y-position is radius*sin(angle).
  MDArray2d makePolarPlotData(Arrayd radii, bool nautical) const {
    assert(periodic);
    assert(_binCount == radii.size());
    const double segmentsPerDegree = 1;
    int segmentsPerBin = int(std::round(segmentsPerDegree*360/_binCount));
    int rowsPerBin = 2 + segmentsPerBin;
    int rows = _binCount*rowsPerBin + 1;
    MDArray2d plotData(rows, 2);
    for (int i = 0; i < _binCount; i++) {
      double left = double(toLeftBound(i).radians());
      double right = double(toRightBound(i).radians());
      HistogramInternal::drawPolarBin(left, right, radii[i],
          plotData.sliceRowBlock(i, rowsPerBin), segmentsPerBin,
          nautical);
    }
    plotData.sliceRow(rows-1).setAll(0);
    return plotData;
  }
 private:
  void init(int binCount, T leftBd, T rightBd, double indexShift = 0) {
    assert(leftBd < rightBd);
    assert(binCount > 0);

    _binCount = binCount;
    _k = (1.0/binCount)*(rightBd - leftBd);
    _m = leftBd - indexShift*_k;
  }

  int _binCount;
  T _k, _m;
};

template <typename T = double>
HistogramMap<Angle<T>, true> makePolarHistogramMap(int binCount,
    double refIndex = 0,
    Angle<T> refAngle = Angle<T>::radians(T(0))) {
  return HistogramMap<Angle<T>, true>(binCount,
      refAngle, refAngle + Angle<T>::degrees(360), refIndex);
}

} /* namespace sail */

#endif /* HISTOGRAM_H_ */
