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


//tm_sec	int	seconds after the minute	0-60*
//tm_min	int	minutes after the hour	0-59
//tm_hour	int	hours since midnight	0-23
//tm_mday	int	day of the month	1-31
//tm_mon	int	months since January	0-11
//tm_year	int	years since 1900
//tm_wday	int	days since Sunday	0-6
//tm_yday	int	days since January 1	0-365
//tm_isdst	int	Daylight Saving Time flag


namespace NavDataConversion {
  TimeStamp makeTimeNmeaFromYMDhms(double yearSince2000, double
      month, double day, double hour,
      double minute, double second) {

    return TimeStamp::UTC(int(yearSince2000 + 2000), int(month), int(day),
              int(hour), int(minute), second);
  }
}



Nav::Nav(MDArray2d row) {
  _flags = 0;

  assert(row.rows() == 1);
  assert(row.cols() == 23 || row.cols() == 22);

  _gpsSpeed = Velocity<double>::knots(row(0, 6));
  _awa = Angle<double>::degrees(row(0, 7));
  _aws = Velocity<double>::knots(row(0, 8));

  _externalTwa = Angle<double>::degrees(row(0, 9));
  _externalTws = Velocity<double>::knots(row(0, 10));

  _magHdg = Angle<double>::degrees(row(0, 11));
  _watSpeed = Velocity<double>::knots(row(0, 12));
  _gpsBearing = Angle<double>::degrees(row(0, 13));


  Angle<double> lat = Angle<double>::degMinMc(row(0, 14), row(0, 15), row(0, 16));
  Angle<double> lon = Angle<double>::degMinMc(row(0, 17), row(0, 18), row(0, 19));
  _pos = GeographicPosition<double>(lon, lat);

  _cwd = row(0, 20); // week day
  _wd = row(0, 21);

  double year = row(0, 0);
  double month = row(0, 1);
  double dayOfTheMonth = row(0, 2);
  double hour = row(0, 3);
  double minute = row(0, 4);
  double second = row(0, 5);
  _time = NavDataConversion::makeTimeNmeaFromYMDhms(year, month, dayOfTheMonth, hour, minute, second);
}

Nav::~Nav() {
  // TODO Auto-generated destructor stub
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

// From load_data.m
//year = 1;
//month = 2;
//dayOfTheMonth = 3;
//hour = 4;
//minute = 5;
//second = 6;
//gpsSpeed = 7;
//awa = 8;
//aws = 9;
//twa = 10;
//tws = 11;
//magHdg = 12;
//watSpeed = 13;
//gpsBearing = 14;
//pos_lat_deg = 15;
//pos_lat_min = 16;
//pos_lat_mc = 17;
//pos_lon_deg = 18;
//pos_lon_min = 19;
//pos_lon_mc = 20;
//cwd = 21;
//wd = 22;
//days = 23;
//days_data = datenum(unsorted_data(:,year) + 2000, unsorted_data(:,month), unsorted_data(:, dayOfTheMonth), unsorted_data(:,hour), unsorted_data(:,minute), unsorted_data(:,second));



Array<Velocity<double> > getExternalTws(Array<Nav> navs) {
  return toArray(map(navs, [&](const Nav &n) {return n.externalTws();}));
}

Array<Angle<double> > getExternalTwa(Array<Nav> navs) {
  return toArray(map(navs, [&](const Nav &n) {return n.externalTwa();}));
}

Array<Velocity<double> > getGpsSpeed(Array<Nav> navs) {
  return toArray(map(navs, [&](const Nav &n) {return n.gpsSpeed();}));
}

Array<Velocity<double> > getWatSpeed(Array<Nav> navs) {
  return toArray(map(navs, [&](const Nav &n) {return n.watSpeed();}));
}

Array<Angle<double> > getGpsBearing(Array<Nav> navs) {
  return toArray(map(navs, [&](const Nav &nav) {
    return nav.gpsBearing();
  }));
}
Array<Angle<double> > getMagHdg(Array<Nav> navs) {
  return toArray(map(navs, [&](const Nav &nav) {
    return nav.magHdg();
  }));
}

Array<Velocity<double> > getAws(Array<Nav> navs) {
  return toArray(map(navs, [&](const Nav &nav) {
    return nav.aws();
  }));
}

Array<Angle<double> > getAwa(Array<Nav> navs) {
  return toArray(map(navs, [&](const Nav &nav) {
    return nav.awa();
  }));
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
