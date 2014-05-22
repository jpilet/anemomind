/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Histogram.h"
#include  <assert.h>
#include <server/common/Span.h>
#include <algorithm>

namespace sail {

HistogramMap::HistogramMap(int count, double leftBd, double rightBd) {
  init(count, leftBd, rightBd);
}


HistogramMap::HistogramMap(int count, Arrayd values) {
  Spand span(values);
  const double marg = 1.0e-9;
  init(count, span.minv() - marg, span.maxv() + marg);
}


int HistogramMap::toBin(double value) const {

  // Floor, to ensure that -0.1 maps to -1
  int index = int(floor(_index2left.inv(value)));

  if (validIndex(index)) {
    assert(toLeftBound(index) <= value);
    assert(value < toRightBound(index));
    return index;
  } else {
    return -1;
  }
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
  for (auto value: values) {
    int index = toBin(value);
    if (validIndex(index)) {
      hist[index]++;
    }
  }
  return hist;
}

Arrayi HistogramMap::assignBins(Arrayd values) const {
  return values.map<int>([&](double x) {return toBin(x);});
}


void HistogramMap::init(int count, double leftBd, double rightBd) {
  assert(leftBd < rightBd);
  assert(count > 0);

  _binCount = count;
  _index2left = LineKM(0, count, leftBd, rightBd);
}

namespace {
  void drawBin(double left, double right, double height, MDArray2d dst) {
    assert(dst.rows() == 3);
    assert(dst.cols() == 2);

    dst(0, 0) = left;
    dst(0, 1) = 0;

    dst(1, 0) = left;
    dst(1, 1) = height;

    dst(2, 0) = right;
    dst(2, 1) = height;
  }
}

MDArray2d HistogramMap::makePlotData(Arrayi counts) const {
  assert(_binCount == counts.size());
  MDArray2d plotData(3*_binCount + 1, 2);
  for (int i = 0; i < _binCount; i++) {
    double left = toLeftBound(i);
    double right = toRightBound(i);
    drawBin(left, right, counts[i], plotData.sliceRowBlock(i, 3));
  }
  int lastRow = plotData.rows()-1;
  plotData(lastRow, 0) = toRightBound(_binCount-1);
  plotData(lastRow, 1) = 0;
  return plotData;
}


} /* namespace sail */
