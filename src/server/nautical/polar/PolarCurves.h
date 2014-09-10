/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARCURVE_H_
#define POLARCURVE_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/Array.h>


namespace sail {

class GnuplotExtra;
class PolarDensity;
class PolarCurves {
 public:
  class Vertex {
   public:
    Vertex() {}
    Vertex(Angle<double> twa_, Velocity<double> bs_) : _twa(twa_), _boatSpeed(bs_) {}
    Velocity<double> x() const;
    Velocity<double> y() const;

    Angle<double> twa() const {
      return _twa;
    }

    Velocity<double> boatSpeed() const {
      return _boatSpeed;
    }
   private:
    Angle<double> _twa;
    Velocity<double> _boatSpeed;
  };

  PolarCurves(Velocity<double> tws, Array<Array<Vertex> > curves) :
    _tws(tws), _curves(curves) {}


  static PolarCurves fromDensity(const PolarDensity &density, Velocity<double> tws,
      int twaCount, Velocity<double> maxBoatSpeed, int bsCount, double quantile);

  void plot(GnuplotExtra *dst);
 private:
  Velocity<double> _tws;
  Array<Array<Vertex> > _curves;
};

} /* namespace mmm */

#endif /* POLARCURVE_H_ */
