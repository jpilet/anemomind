/*
 *  Created on: 2014-03-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  A periodic histogram (with a period of Angle<double>::radians(2.0*M_PI))
 */

#ifndef PERIODICHIST_H_
#define PERIODICHIST_H_

#include <server/common/LineKM.h>
#include <server/common/Array.h>
#include <server/common/MDArray.h>
#include <server/common/PhysicalQuantity.h>

namespace sail {

class PeriodicHistIndexer {
 public:
  PeriodicHistIndexer() : _binCount(0) {}
  PeriodicHistIndexer(int binCount, double shift = 0.5/*Middle of the first bin at 0*/);

  int toBin(Angle<double> angle) const;
  Angle<double> binLeft(int binIndex) const;
  Angle<double> binMiddle(int binIndex) const;
  Angle<double> binRight(int binIndex) const;
  bool initialized()  const {return _binCount > 0;}
  int count() const {return _binCount;}
  bool operator==(const PeriodicHistIndexer &other) const;
 private:
  bool isValidBinIndex(int binIndex) const;
  int _binCount;
  LineKM _binMap;
};

class PeriodicHist {
 public:
  PeriodicHist(int binCount, double shift = 0.5);
  PeriodicHist(PeriodicHistIndexer indexer, Arrayd sum) :
    _indexer(indexer), _sum(sum) {}

  void add(Angle<double> angle, double val);
  double value(Angle<double> angle);
  double valueInBin(int binIndex) {return _sum[binIndex];}
  MDArray2d makePolarPlotData();
  const PeriodicHistIndexer &indexer() const {return _indexer;}
  Arrayd data() const {return _sum;}
 private:
  PeriodicHistIndexer _indexer;
  Arrayd _sum;
};

// Useful to compute averages
PeriodicHist operator/ (const PeriodicHist &a, const PeriodicHist &b);

}

#endif /* PERIODICHIST_H_ */
