/*
 *  Created on: 2014-03-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PeriodicHist.h"
#include <cmath>
#include <assert.h>
#include <server/common/math.h>

namespace sail {

PeriodicHistIndexer::PeriodicHistIndexer(int binCount, double shift) :
  _binCount(binCount), _binMap(LineKM(0.0 + shift, binCount + shift, 0.0, 2.0*M_PI)) {
}

int PeriodicHistIndexer::toBin(Angle<double> angle) const {
  assert(initialized());
  int index0 = int(floor(_binMap.inv(angle.radians())));
  int index = posMod(index0, _binCount);
  assert(isValidBinIndex(index));
  return index;
}

Angle<double> PeriodicHistIndexer::binLeft(int binIndex) const {
  assert(isValidBinIndex(binIndex));
  return Angle<double>::radians(_binMap(binIndex));
}

bool PeriodicHistIndexer::isValidBinIndex(int binIndex) const {
  return 0 <= binIndex && binIndex < _binCount;
}

Angle<double> PeriodicHistIndexer::binRight(int binIndex) const {
  assert(isValidBinIndex(binIndex));
  return Angle<double>::radians(_binMap(binIndex + 1.0));
}

bool PeriodicHistIndexer::operator==(const PeriodicHistIndexer &other) const {
  return _binMap == other._binMap && _binCount == other._binCount;
}

Angle<double> PeriodicHistIndexer::binMiddle(int binIndex) const {
  assert(isValidBinIndex(binIndex));
  return Angle<double>::radians(_binMap(binIndex + 0.5));
}

PeriodicHist::PeriodicHist(int binCount, double shift) :
    _indexer(binCount, shift), _sum(binCount) {
    _sum.setTo(0.0);
}

void PeriodicHist::add(Angle<double> angle, double val) {
  _sum[_indexer.toBin(angle)] += val;
}

double PeriodicHist::value(Angle<double> angle) {
  return _sum[_indexer.toBin(angle)];
}

namespace {
  void setPolarPos(Angle<double> a, double value, MDArray2d &dst, int row) {
    dst(row, 0) = value*cos(a);
    dst(row, 1) = value*sin(a);
  }
}

MDArray2d PeriodicHist::makePolarPlotData() {
  int count = _indexer.count();
  MDArray2d data(3*count + 1, 2);
  data.setAll(0.0);
  for (int i = 0; i < count; i++) {
    MDArray2d subdata = data.sliceRowBlock(i, 3).sliceRowsFrom(1);
    double value = _sum[i];
    setPolarPos(_indexer.binLeft(i), value, subdata, 0);
    setPolarPos(_indexer.binRight(i), value, subdata, 1);
  }
  return data;
}

PeriodicHist operator/ (const PeriodicHist &a, const PeriodicHist &b) {
  assert(a.indexer() == b.indexer());
  Arrayd adata = a.data();
  Arrayd bdata = b.data();
  int count = adata.size();
  assert(count == bdata.size());
  Arrayd result(count);
  for (int i = 0; i < count; i++) {
    result[i] = adata[i]/bdata[i];
  }
  return PeriodicHist(a.indexer(), result);
}


}
