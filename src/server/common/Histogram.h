/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <server/common/Array.h>
#include <server/common/LineKM.h>
#include <server/common/MDArray.h>
#include <server/common/Span.h>
#include <server/common/math.h>

namespace sail {

namespace HistogramInternal {
  template <typename T>
  Array<Array<T> > allocateGroups(Arrayi cpb) {
    return cpb.map<Array<T> >([&](int i) {return Array<T>(i);});
  }

  void drawBin(double left, double right, double height, MDArray2d dst);

  template <typename T>
  std::function<double(T)> makeDoubleMapper() {
    return std::function<double(T)>();
  }

  template <>
  inline std::function<double(double)> makeDoubleMapper<double>() {
    return [](double x) {return x;};
  }
}

template <typename T, bool periodic>
class HistogramMap {
 public:
  HistogramMap() : _binCount(0) {}

  HistogramMap(int count, T leftBd, T rightBd) {
    assert(!periodic);
    init(count, leftBd, rightBd);
  }

  HistogramMap(int count, Array<T> values) {
    Span<T> span(values);
    const double marg = 1.0e-9;
    init(count, span.minv() - marg, span.maxv() + marg);
  }

  int binCount() const {return _binCount;}

  int toBin(T value) const {
    int index = int(floor((value - _m)/_k));
    if (periodic) {
      return positiveMod(index, _binCount);
    } else if (validIndex(index)) {
      return index;
    } else {
      return -1;
    }
  }

  T toLeftBound(double binIndex) const {
    return binIndex*_k + _m;
  }

  double toRightBound(double binIndex) const {
    return toLeftBound(binIndex + 1.0);
  }

  double toCenter(double binIndex) const {
    return toLeftBound(binIndex + 0.5);
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

  Arrayi assignBins(Arrayd values) const {
    return values.map<int>([&](double x) {return toBin(x);});
  }

  bool defined() const {return _binCount > 0;}
  bool undefined() const {return !defined();}

  // Returns true if the index refers to a valid element in
  // the target array. Therefore, it applies to both
  // periodic and non-periodic histograms.
  bool validIndex(int index) const {
    return 0 <= index && index < _binCount;
  }

  MDArray2d makePlotData(Arrayi counts,
      std::function<double(T)> mapper = HistogramInternal::makeDoubleMapper<T>()) const {
    assert(_binCount == counts.size());
    MDArray2d plotData(3*_binCount + 1, 2);
    for (int i = 0; i < _binCount; i++) {
      double left = mapper(toLeftBound(i));
      double right = mapper(toRightBound(i));
      HistogramInternal::drawBin(left, right, counts[i], plotData.sliceRowBlock(i, 3));
    }
    int lastRow = plotData.rows()-1;
    plotData(lastRow, 0) = mapper(toRightBound(_binCount-1));
    plotData(lastRow, 1) = 0;
    return plotData;
  }
 private:
  void init(int binCount, T leftBd, T rightBd) {
    assert(leftBd < rightBd);
    assert(binCount > 0);

    _binCount = binCount;
    _k = (1.0/binCount)*(rightBd - leftBd);
    _m = leftBd;
  }

  int _binCount;
  T _k, _m;
};

} /* namespace sail */

#endif /* HISTOGRAM_H_ */
