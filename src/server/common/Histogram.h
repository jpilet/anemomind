/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <server/common/Array.h>
#include <server/common/LineKM.h>
#include <server/common/MDArray.h>

namespace sail {

namespace HistogramInternal {
  template <typename T>
  Array<Array<T> > allocateGroups(Arrayi cpb) {
    return cpb.map<Array<T> >([&](int i) {return Array<T>(i);});
  }
}

class HistogramMap {
 public:
  HistogramMap() : _binCount(0) {}
  HistogramMap(int count, double leftBd, double rightBd);
  HistogramMap(int count, Arrayd values);
  int binCount() const {return _binCount;}
  int toBin(double value) const;
  double toLeftBound(int binIndex) const;
  double toRightBound(int binIndex) const;
  double toCenter(int binIndex) const;
  Arrayi countPerBin(Arrayd values) const;
  Arrayi assignBins(Arrayd values) const;
  bool defined() const {return _binCount > 0;}
  bool undefined() const {return !defined();}

  bool validIndex(int index) const {
    return 0 <= index && index < _binCount;
  }

  template <typename T>
  Array<Array<T> > groupValuesByBin(Arrayd X, Array<T> Y) const {
    Arrayi cpb = countPerBin(X);
    Array<Array<T> > groups = HistogramInternal::allocateGroups<T>(cpb);
    assert(groups.size() == _binCount);
    Arrayi counters = Arrayi::fill(_binCount, 0);
    int n = X.size();
    assert(n == Y.size());
    for (int i = 0; i < n; i++) {
      int index = toBin(X[i]);
      groups[index][counters[index]] = Y[i];
      counters[index]++;
    }
    return groups;
  }

  MDArray2d makePlotData(Arrayi counts) const;
 private:
  void init(int binCount, double leftBd, double rightBd);
  int _binCount;
  LineKM _index2left;
};

} /* namespace sail */

#endif /* HISTOGRAM_H_ */
