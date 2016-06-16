/*
 * Nav.cpp
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#include "Nav.h"
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <algorithm>
#include <ctime>
#include <server/common/string.h>
#include <server/common/PhysicalQuantityIO.h>
#include <server/common/logging.h>
#include <server/common/Functional.h>



namespace sail {

Nav::Nav() : _time(TimeStamp::makeUndefined()) {
  _flags = 0;
  _cwd = -1;
  _wd = -1;
}

bool Nav::operator== (const Nav &other) const {
  constexpr double marg = 1.0e-12; // Set to 0 if we don't tolerate any deviation.

  return _gpsSpeed.nearWithNan(other._gpsSpeed, marg) &&
      _awa.nearWithNan(other._awa, marg) &&
      _aws.nearWithNan(other._aws, marg) &&
      _boatId == other._boatId &&
      _externalTwa.nearWithNan(other._externalTwa, marg) &&
      _externalTws.nearWithNan(other._externalTws, marg) &&
      _magHdg.nearWithNan(other._magHdg, marg) &&
      _watSpeed.nearWithNan(other._watSpeed, marg) &&
      _gpsBearing.nearWithNan(other._gpsBearing, marg) &&
      _pos.lon().nearWithNan(other._pos.lon(), marg) &&
      _pos.lat().nearWithNan(other._pos.lat(), marg) &&
      _pos.alt().nearWithNan(other._pos.alt(), marg) &&
      _trueWind[0].nearWithNan(other._trueWind[0], marg) &&
      _trueWind[1].nearWithNan(other._trueWind[1], marg) &&
      (nearWithNan(_cwd, other._cwd, marg)) && (nearWithNan(_wd, other._wd, marg));
}

HorizontalMotion<double> Nav::gpsMotion() const {
  return HorizontalMotion<double>::polar(gpsSpeed(), gpsBearing());
}

Nav::Id Nav::id() const {
  if (hasId()) {
    int64_t time = _time.toMilliSecondsSince1970();
    static_assert(sizeof(time) == 8, "The size of the time datatype seems to have changed.");
    return _boatId + int64ToHex(time);
  } else {
    return "";
  }
}

bool Nav::hasId() const {
  return hasBoatId() && _time.defined();
}


Array<Velocity<double> > getGpsSpeed(Array<Nav> navs) {
  return toArray(map(navs, [&](const Nav &n) {return n.gpsSpeed();}));
}

HorizontalMotion<double> Nav::estimateTrueWind() const {
    double defaultParams[TrueWindEstimator::NUM_PARAMS];
    TrueWindEstimator::initializeParameters(defaultParams);
    return TrueWindEstimator::computeTrueWind(defaultParams, *this);
}

Angle<double> Nav::bestTwaEstimate() const {
  if (hasTrueWindOverGround()) {
    return twaFromTrueWindOverGround();
  }
  if (hasExternalTrueWind()) {
    return externalTwa();
  }
  if (hasApparentWind()) {
    return calcTwa(estimateTrueWind(), gpsBearing());
  }
  return Angle<>();
}

std::ostream &operator<<(std::ostream &s, const Nav &x) {
  s << "Nav:\n";
  s << "  time: " << x.time() << "\n";
  s << "  maghdg: " << x.magHdg() << "\n";
  s << "  aws: " << x.aws() << "\n";
  s << "  awa: " << x.awa() << "\n";
  s << "  watspeed: " << x.watSpeed() << "\n";
  s << "  gps bearing: " << x.gpsBearing() << "\n";
  s << "  gps speed: " << x.gpsSpeed() << "\n";
  s << "  ext twa: " << x.externalTwa() << "\n";
  s << "  ext twdir: " << x.externalTwdir() << "\n";
  s << "  ext tws: " << x.externalTws() << "\n";
  s << "  twa: " << x.deviceTwa() << "\n";
  s << "  twdir: " << x.deviceTwdir() << "\n";
  s << "  tws: " << x.deviceTws() << "\n";
  s << "  lat: " << x.geographicPosition().lat() << "\n";
  s << "  lon: " << x.geographicPosition().lon() << "\n";
  s << "  alt: " << x.geographicPosition().alt() << "\n";
  return s;
}

} /* namespace sail */
