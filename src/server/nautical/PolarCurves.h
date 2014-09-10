/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARCURVE_H_
#define POLARCURVE_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/Array.h>

namespace sail {

class PolarCurves {
 public:
  class Point {
   public:
    Point(Angle<double> twa, Velocity<double> bs) : _twa(twa), _boatSpeed(bs) {}
   private:
    Angle<double> _twa;
    Velocity<double> _boatSpeed;
  };

  PolarCurves(Velocity<double> tws, Array<Array<Point> > curves) :
    _tws(tws), _curves(curves) {}
 private:
  Velocity<double> _tws;
  Array<Array<Point> > _curves;
};

} /* namespace mmm */

#endif /* POLARCURVE_H_ */
