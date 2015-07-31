/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_TGTSPEED_TARGETSPEEDPOINT_H_
#define SERVER_NAUTICAL_TGTSPEED_TARGETSPEEDPOINT_H_

#include <server/common/Array.h>
#include <server/common/PhysicalQuantityIO.h>
#include <device/Arduino/libraries/CalibratedNav/CalibratedNav.h>

namespace sail {


class VmgData {
 public:
  VmgData() : _stability(-1) {}
  VmgData(Velocity<double> windSpeed, Velocity<double> vmg, double stability) :
    _windSpeed(windSpeed), _vmg(vmg), _stability(stability) {}

  Velocity<double> windSpeed() const {
    return _windSpeed;
  }

  Velocity<double> vmg() const {
    return _vmg;
  }

  double stability() const {
    return _stability;
  }
 private:
  Velocity<double> _windSpeed, _vmg;
  double _stability;
};

class TargetSpeedPoint {
 public:
  TargetSpeedPoint() {
    _defined = false;
  }

  VmgData calcVmg(Angle<double> targetTwa) const {
    return VmgData(_windSpeed, cos(_windAngle - targetTwa)*_boatSpeed, _stability());
  }

  TargetSpeedPoint(Velocity<double> boatSpeed, Velocity<double> windSpeed,
      Angle<double> windAngle, double stability) : _defined(true),
          _windSpeed(windSpeed), _boatSpeed(boatSpeed), _windAngle(windAngle), _stability(stability) {}

  TargetSpeedPoint(const CalibratedNav<double> &cnav) {
    _defined = false;
    auto boatMotion = cnav.boatMotionOverWater.otherwise(cnav.gpsMotion);
    if (boatMotion.defined()) {
      _boatSpeed = boatMotion().norm();
      auto wind = cnav.trueWindOverWater().otherwise(cnav.trueWindOverGround);
      if (wind.defined()) {
        _windSpeed = wind().norm();
        auto boatOrientation = cnav.boatOrientation.otherwise(
            cnav.gpsMotion.applyFun<Angle<double> >([=](const HorizontalMotion<double> &x) {
              return x.angle();
            }));
        if (boatOrientation.defined()) {
          _windAngle = (wind().angle() + Angle<double>::degrees(180)) - boatOrientation();
          _defined = true;
        }
      }
    }
  }

  bool defined() const {
    return _defined;
  }


  Velocity<double> boatSpeed() const {
    assert(_defined);
    return _boatSpeed;
  }
  Velocity<double> windSpeed() const {
    assert(_defined);
    return _windSpeed;
  }
  Angle<double> windAngle() const {
    assert(_defined);
    return _windAngle;
  }

  HorizontalMotion<double> wind() const {
    assert(_defined);
    return HorizontalMotion<double>::polar(_windSpeed, _windAngle);
  }

  DefinedValue<double> stability() const {
    assert(_defined);
    return _stability;
  }

  void setStability(double s) {
    _stability.set(s);
  }
 private:
  bool _defined;
  Velocity<double> _boatSpeed;
  Velocity<double> _windSpeed;
  Angle<double> _windAngle;
  DefinedValue<double> _stability;
};

Array<Velocity<double> > getBoatSpeeds(Array<TargetSpeedPoint> x);
Array<Velocity<double> > getWindSpeeds(Array<TargetSpeedPoint> x);
Array<Angle<double> > getWindAngles(Array<TargetSpeedPoint> x);
Array<double> getStabilities(Array<TargetSpeedPoint> x);
Array<TargetSpeedPoint> computeStabilities(Array<TargetSpeedPoint> src);




}




#endif /* SERVER_NAUTICAL_TGTSPEED_TARGETSPEEDPOINT_H_ */
