/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARPOINT_H_
#define POLARPOINT_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/math/PolarCoordinates.h>

namespace sail {

/*
 * A point in a polar diagram.
 */
class PolarPoint {
 public:
  PolarPoint() : _navIndex(-1) {}
  PolarPoint(Velocity<double> tws_,
        Angle<double> twa_,
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

  bool hasNaN() const {
    return _twa.isNaN() || _tws.isNaN() || _boatSpeed.isNaN();
  }

  Velocity<double> x() const {
    return calcPolarX(true, _boatSpeed, _twa);
  }

  Velocity<double> y() const {
    return calcPolarY(true, _boatSpeed, _twa);
  }
 private:
  int _navIndex;
  Angle<double> _twa;
  Velocity<double> _tws;
  Velocity<double> _boatSpeed;
};


}




#endif /* POLARPOINT_H_ */
