/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BASICPOLAR_H_
#define BASICPOLAR_H_

#include <server/common/Histogram.h>

namespace sail {

class Polar2d {
 private:
  typedef HistogramMap<double, true> PolarHistMap;
 public:

  Polar2d() {}
  Polar2d(PolarHistMap map) : _histmap(map) {}
 private:
  PolarHistMap _histmap;
};

class BasicPolar {
 public:
  BasicPolar();
  ~BasicPolar();
 private:
  HistogramMap<double, false> _histmap;
  Array<Polar2d> _slices;
};

}

#endif /* BASICPOLAR_H_ */
