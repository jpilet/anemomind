/*
 *  Created on: 2014-11-06
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATEDNAV_H_
#define CALIBRATEDNAV_H_

#include "../PhysicalQuantity/PhysicalQuantity.h"
#include <cassert>

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
 */
template <typename T>
class CalibratedNav {
 public:
  template <typename InstrumentAbstraction>

  // Angle accessors
  Angle<T> rawAwa() const {return read(_rawAwa);}
  void setRawAwa(Angle<T> value) {_rawAwa = value;}
  Angle<T> calibAwa() const {return read(_calibAwa);}
  void setCalibAwa(Angle<T> value) {_calibAwa = value;}

  Angle<T> rawMagHdg() const {return read(_rawMagHdg);}
  void setRawMagHdg(Angle<T> value) {_rawMagHdg = value;}
  Angle<T> calibMagHdg() const {return read(_calibMagHdg);}
  void setCalibMagHdg(Angle<T> value) {_calibMagHdg = value;}

  Angle<T> rawGpsBearing() const {return read(_rawGpsBearing);}
  void setRawGpsBearing(Angle<T> value) {_rawGpsBearing = value;}
  Angle<T> calibGpsBearing() const {return read(_calibGpsBearing);}
  void setCalibGpsBearing(Angle<T> value) {_calibGpsBearing = value;}
 private:
  // Helper function that checks that we don't read uninitialized values in debug mode.
  template <typename S>
  static S read(S src) {
    assert(!src.isNaNOrFalse());
    return src;
  }

  Angle<T> _rawAwa, _calibAwa,
    _rawMagHdg, _calibMagHdg,
    _rawGpsBearing, _calibGpsBearing;
  Velocity<T> _rawAws, _calibAws,
    _rawWatSpeed, _calibWatSpeed,
    _rawGpsSpeed, _calibGpsSpeed;
};

}


#endif /* CALIBRATEDNAV_H_ */
