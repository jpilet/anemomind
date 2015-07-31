/*
 *  Created on: 2014-11-06
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATEDNAV_H_
#define CALIBRATEDNAV_H_

#include "../PhysicalQuantity/PhysicalQuantity.h"
#include <cassert>

namespace sail {

// More powerful and safe than using nan
// to indicate whether a value is defined or not.
template <typename T>
class DefinedValue {
 public:
  DefinedValue() : _defined(false) {}
  DefinedValue(T x) : _defined(true), _value(x) {}

  // If a there is a public field
  // in a class of type DefinedValue,
  // calling this operator on that field
  // gives the feel of calling an accessor of the class.
  T operator()() const {
    assert(_defined); // <-- only active in debug mode.
    return _value;
  }

  T get(T defaultValue) const {
    return (_defined? _value : defaultValue);
  }

  void set(T x) {
    _defined = true;
    _value = x;
  }

  void setOnce(T x) {
    assert(!_defined);
    set(x);
  }

  template <typename S>
  DefinedValue<S> applyFun(std::function<S(T)> fun) const {
    if (_defined) {
      return DefinedValue<S>(fun(_value));
    }
    return DefinedValue<S>();
  }

  template <typename S>
  DefinedValue<S> applyFun(std::function<S(T, T)> fun, const DefinedValue<S> &other) const {
    if (_defined && other._defined) {
      return DefinedValue<S>(fun(_value, other._value));
    }
    return DefinedValue<S>();
  }

  DefinedValue<T> operator+(const DefinedValue<T> &other) const {
    return applyFun<T>([&](T a, T b) {return a + b;}, other);
  }

  DefinedValue<T> operator-(const DefinedValue<T> &other) const {
    return applyFun<T>([&](T a, T b) {return a - b;}, other);
  }

  bool defined() const {return _defined;}
  bool undefined() const {return !_defined;}

  const DefinedValue<T> &otherwise(const DefinedValue<T> &other) const {
    if (_defined) {
      return *this;
    }
    return other;
  }
 private:
  bool _defined;
  T _value;
};



/*
 * The purpose of this class
 * is to hold the results of a
 * calibration procedure.
 *
 * It is initialized from an InstrumentAbstraction
 * (such as a Nav) and the idea is then that some
 * calibration procedure (not implemented here) populates
 * it with calibrated values.
 *
 * Note that this class does not exhibit the
 * the InstrumentAbstraction type of methods (awa(), aws(), etc...)
 * because for each value, there is a raw and calibrated one. However,
 * its constructors accepts an argument that follows the InstrumentAbstracation
 * convention.
 *
 *
 * How to use this class:
 *  1. Construct an instance from an object exhibiting the InstrumentAbstraction interface.
 *  2. Populate the calibrated instance variables any way you like.
 *  3. Call the fill() method to compute true wind, current, etc, using the calibrated values.
 *
 * See CorrectorSet::calibrate for an example of how it is used.
 */
template <typename T>
class CalibratedNav {
 public:
  CalibratedNav() {}

  typedef DefinedValue<Angle<T> > DefinedAngle;
  typedef DefinedValue<Velocity<T> > DefinedVelocity;
  typedef DefinedValue<HorizontalMotion<T> > DefinedMotion;

  // InstrumentAbstraction can for instance be a Nav.
  template <typename InstrumentAbstraction>
  CalibratedNav(const InstrumentAbstraction &x) :
    rawAwa(x.awa()), rawMagHdg(x.magHdg()),
    rawAws(x.aws()), rawWatSpeed(x.watSpeed()),
    gpsMotion(HorizontalMotion<T>::polar(x.gpsSpeed(), x.gpsBearing())),
    driftAngle(Angle<T>::degrees(T(0))) {}

  /*
   * Since all instance variables are encapsulated
   * in the type DefinedValue, I think we can make
   * them public instead of having them private.
   */
  // Values that are populated by the constructor.
  DefinedMotion gpsMotion;
  DefinedAngle rawAwa, rawMagHdg;
  DefinedVelocity rawAws, rawWatSpeed;

  // Values that need to be calibrated externally.
  DefinedAngle calibAwa, boatOrientation;
  DefinedVelocity calibAws, calibWatSpeed;
  DefinedAngle driftAngle; // <-- Optional to calibrate.

  // Depend on the calibrated values.
  DefinedAngle  directionApparentWindBlowsTo;
  DefinedMotion apparentWind;
  DefinedMotion trueWindOverGround;
  DefinedMotion trueCurrentOverGround;
  DefinedMotion boatMotionOverWater;

  /*
   *
   * Extra nice-to-have accessors
   */
  DefinedValue<Angle<T> > twdirOverGround() const {
    if (trueWindOverGround.defined()) {
      return trueWindOverGround().angle() + Angle<T>::degrees(T(180));
    }
    return DefinedValue<Angle<T> >();
  }

  DefinedValue<Velocity<T> > twsOverGround() const {
    if (trueWindOverGround.defined()) {
      return trueWindOverGround().norm();
    }
    return DefinedValue<Velocity<T> >();
  }

  DefinedValue<HorizontalMotion<T> > motionRelativeToBoat(const HorizontalMotion<T> &x) const {
    if (gpsMotion.defined()) {
      return x - gpsMotion();
    }
    return DefinedValue<HorizontalMotion<T> >();
  }

  DefinedValue<Angle<T> > angleRelativeToBoat(const Angle<T> &x) const {
    if (boatOrientation.defined()) {
      return x - boatOrientation();
    }
    return DefinedValue<Angle<T> >();
  }

  DefinedValue<HorizontalMotion<T> > trueWindOverWater() const {
    return trueWindOverGround - trueCurrentOverGround;
  }
};

}


#endif /* CALIBRATEDNAV_H_ */
