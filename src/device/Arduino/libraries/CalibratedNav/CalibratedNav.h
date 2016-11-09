/*
 *  Created on: 2014-11-06
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATEDNAV_H_
#define CALIBRATEDNAV_H_

#include "../PhysicalQuantity/PhysicalQuantity.h"
#include <cassert>
#include <functional>
#include <server/common/Optional.h>
#include <server/common/numerics.h>
#include <server/nautical/NavCompatibility.h>

namespace sail {

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

  typedef Optional<Angle<T> > DefinedAngle;
  typedef Optional<Velocity<T> > DefinedVelocity;
  typedef Optional<HorizontalMotion<T> > DefinedMotion;

  // InstrumentAbstraction can for instance be a Nav.
  template <typename InstrumentAbstraction>
  CalibratedNav(const InstrumentAbstraction &x) :
    gpsMotion(HorizontalMotion<T>::polar(x.gpsSpeed(), x.gpsBearing())),
    rawAwa(x.awa()), rawMagHdg(x.magHdg()),
    rawAws(x.aws()), rawWatSpeed(x.watSpeed()),
    driftAngle(Angle<T>::degrees(T(0))) {}

  bool hasNan() const {
    return isNaN(rawAwa) ||
        isNaN(rawAws) ||
        isNaN(rawMagHdg) ||
        isNaN(rawWatSpeed) ||
        isNaN(driftAngle) ||
        isNaN(calibWatSpeed) ||
        isNaN(calibAws) ||
        isNaN(calibAwa) ||
        isNaN(boatOrientation);
  }

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
  Optional<Angle<T> > twdirOverGround() const {
    if (trueWindOverGround.defined()) {
      return trueWindOverGround().angle() + Angle<T>::degrees(T(180));
    }
    return Optional<Angle<T> >();
  }

  Optional<Velocity<T> > twsOverGround() const {
    if (trueWindOverGround.defined()) {
      return trueWindOverGround().norm();
    }
    return Optional<Velocity<T> >();
  }

  Optional<HorizontalMotion<T> > motionRelativeToBoat(const HorizontalMotion<T> &x) const {
    if (gpsMotion.defined()) {
      return x - gpsMotion();
    }
    return Optional<HorizontalMotion<T> >();
  }

  Optional<Angle<T> > angleRelativeToBoat(const Angle<T> &x) const {
    if (boatOrientation.defined()) {
      return x - boatOrientation();
    }
    return Optional<Angle<T> >();
  }

  Optional<HorizontalMotion<T> > trueWindOverWater() const {
    return trueWindOverGround - trueCurrentOverGround;
  }
};

// An abstract class used for evaluation of calibration algorithms.
class CorrectorFunction {
 public:
  virtual Array<CalibratedNav<double> > operator()(const NavDataset &navs) const = 0;
  virtual std::string toString() const = 0;
  virtual ~CorrectorFunction() {}

  typedef std::shared_ptr<CorrectorFunction> Ptr;
};


}


#endif /* CALIBRATEDNAV_H_ */
