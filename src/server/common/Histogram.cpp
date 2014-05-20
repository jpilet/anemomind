/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Histogram.h"
#include  <assert.h>

namespace sail {

HistogramMap::HistogramMap(int count, double leftBd, double rightBd) {
  init(count, leftBd, rightBd);
}


HistogramMap::HistogramMap(int count, Arrayd values) {
  double minv = values[0];
  double maxv = minv;
  int n = values.size();
  for (int i = 1; i < n; i++) {
    double x = values[i];
    minv = std::min(minv, x);
    maxv = std::max(maxv, x);
  }
  const double marg = 1.0e-9;
  init(count, minv - marg, maxv + marg);
}


int HistogramMap::toBin(double value) const {
  int index = int(_index2left.inv(value));
  assert(validIndex(index));
  return index;
}

double HistogramMap::toLeftBound(int binIndex) const {
  assert(validIndex(binIndex));
  return _index2left(binIndex);
}

double HistogramMap::toRightBound(int binIndex) const {
  assert(validIndex(binIndex));
  return _index2left(binIndex + 1.0);
}

double HistogramMap::toCenter(int binIndex) const {
  assert(validIndex(binIndex));
  return _index2left(binIndex + 0.5);
}
Arrayi HistogramMap::countPerBin(Arrayd values) const {
  Arrayi hist(_binCount);
  hist.setTo(0);
  int count = values.size();
  for (int i = 0; i < count; i++) {
    hist[toBin(values[i])]++;
  }
  return hist;
}

Arrayi HistogramMap::assignBins(Arrayd values) const {
  return values.map<int>([&](double x) {return toBin(x);});
}


void HistogramMap::init(int count, double leftBd, double rightBd) {
  _binCount = count;
  _index2left = LineKM(0, count, leftBd, rightBd);
}


} /* namespace sail */
