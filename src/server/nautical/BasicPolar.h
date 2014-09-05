/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BASICPOLAR_H_
#define BASICPOLAR_H_

#include <server/common/Histogram.h>

namespace sail {

/*
 * A point in a polar diagram.
 */
class PolarPoint {
 public:
  PolarPoint() : _navIndex(-1) {}
  PolarPoint(Angle<double> twa_,
      Velocity<double> tws_,
      Velocity<double> boatSpeed_,

      int navIndex = -1); // Optional reference to the Nav in an array, from which Nav these values were obtained.

  int navIndex() const {
    return _navIndex;
  }

  Angle<double> twa() const {
    return _twa;
  }

  Velocity<double> tws() const {
    return _tws;
  }

  Velocity<double> boatSpeed() const {
    return _boatSpeed;
  }

  bool operator< (const PolarPoint &other) const {
    return _boatSpeed < other._boatSpeed;
  }
 private:
  int _navIndex;
  Angle<double> _twa;
  Velocity<double> _tws;
  Velocity<double> _boatSpeed;
};

class Polar2d {
 private:
  typedef HistogramMap<Angle<double>, true> PolarHistMap;
 public:
  Polar2d() {}
  Polar2d(PolarHistMap map,
    Array<PolarPoint> points);
 private:
  PolarHistMap _histmap;
  Array<Array<PolarPoint> > _pointsPerBin;
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
