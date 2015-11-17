/*
 * Nav.h
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#ifndef NAV_H_
#define NAV_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <device/Arduino/NMEAStats/test/ScreenRecordingSimulator.h>
#include <string>
#include <server/common/MDArray.h>
#include <server/common/math.h>
#include <server/nautical/GeographicPosition.h>
#include <server/common/TimeStamp.h>

namespace sail {

// Helper functions to convert from the format of
// NmeaParser
namespace NavDataConversion {
  TimeStamp makeTimeNmeaFromYMDhms(double yearSince2000, double month, double day, double hour, double minute, double second);
}

// Represents a single recording of data from the devices onboard.
class Nav {
 public:
  typedef std::string Id; // We may want to replace this typedef by a special type for Id's
  typedef double type;
  static Nav::Id debuggingBoatId() {return "FFFFFFFF";}

  Nav();
  Nav(TimeStamp ts) : _flags(0), _cwd(-1), _wd(-1), _time(ts) { }
  Nav(MDArray2d row);
  virtual ~Nav();

  // For sorting
  bool operator< (const Nav &other) const {
    return _time < other._time;
  }

  TimeStamp time() const {
    return _time;
  }

  const GeographicPosition<double> &geographicPosition() const {return _pos;}
  GeographicPosition<double> &geographicPosition() {return _pos;}
  Angle<double> awa() const {return _awa;}
  Velocity<double> aws() const {return _aws;}
  bool hasApparentWind() const { return !isnan(_awa) && !isnan(_aws); }

  Angle<double> magHdg() const {return _magHdg;}
  bool hasMagHdg() const { return !isnan(_magHdg); }

  Angle<double> gpsBearing() const {return _gpsBearing;}
  Velocity<double> gpsSpeed() const {return _gpsSpeed;}

  Velocity<double> watSpeed() const {return _watSpeed;}
  bool hasWatSpeed() const { return !isnan(_watSpeed); }

  Angle<double> externalTwa() const {return _externalTwa;}
  Velocity<double> externalTws() const {return _externalTws;}
  bool hasExternalTrueWind() const {
    return !isnan(_externalTwa) && !isnan(_externalTws); }

  HorizontalMotion<double> gpsMotion() const;

  // As computed by the calibrated model. Not always available.
  HorizontalMotion<double> trueWindOverGround() const { return _trueWind; }
  bool hasTrueWindOverGround() const { return !isnan(_trueWind[0]); }

  Angle<double> twdir() const {
    return Angle<double>::degrees(180) + trueWindOverGround().angle();
  }
  
  Angle<double> externalTwdir() const {
    return externalTwa() + Angle<double>::degrees(180) + gpsBearing();
  }

  Angle<double> twaFromTrueWindOverGround() const {
    return twdir() - gpsBearing();
  }

  void setAwa(Angle<double> awa_) {_awa = awa_;}
  void setAws(Velocity<double> aws_) {_aws = aws_;}
  void setMagHdg(Angle<double> magHdg_) {_magHdg = magHdg_;}
  void setGpsBearing(Angle<double> gpsBearing_) {_gpsBearing = gpsBearing_;}
  void setGpsSpeed(Velocity<double> gpsSpeed_) {_gpsSpeed = gpsSpeed_;}
  void setWatSpeed(Velocity<double> watSpeed_) {_watSpeed = watSpeed_;}
  void setTime(const TimeStamp &t) {_time = t;}
  void setGeographicPosition(GeographicPosition<double> pos) {_pos = pos;}

  void setExternalTwa(Angle<double> twa_) {_externalTwa = twa_;}
  void setExternalTws(Velocity<double> tws_) {_externalTws = tws_;}

  void setTrueWindOverGround(const HorizontalMotion<double>& trueWind) { _trueWind = trueWind; }

  bool operator== (const Nav &other) const;

  void setBoatId(const Id &bi) {_boatId = bi;}
  bool hasBoatId() const {return !_boatId.empty();}

  // TODO: Require this method to return true before a Nav is inserted to a database.
  bool isIndexed() const {return hasId() && hasBoatId();}

  Id id() const;
  bool hasId() const;
  const Id &boatId() const {return _boatId;}

  void setDeviceScreen(ScreenInfo info) {
    _flags |= DEVICE_SCREEN; _deviceScreen = info; }
  ScreenInfo deviceScreen() const { return _deviceScreen; }
  bool hasDeviceScreen() const { return _flags & DEVICE_SCREEN; }

  bool hasDeviceTargetVmg() const { return _flags & DEVICE_TARGET_VMG; }
  Velocity<double> deviceTargetVmg() const { return _deviceTargetVmg; }
  void setDeviceTargetVmg(Velocity<double> p) { _deviceTargetVmg= p; _flags |= DEVICE_TARGET_VMG; }

  bool hasDeviceVmg() const { return _flags & DEVICE_VMG; }
  Velocity<double> deviceVmg() const { return _deviceVmg; }
  void setDeviceVmg(Velocity<double> p) { _deviceVmg = p; _flags |= DEVICE_VMG; }

  bool hasDeviceTws() const { return _flags & DEVICE_TWS; }
  Velocity<double> deviceTws() const { return _deviceTws; }
  void setDeviceTws(Velocity<double> p) { _deviceTws = p; _flags |= DEVICE_TWS; }

  bool hasDeviceTwdir() const { return _flags & DEVICE_TWDIR; }
  Angle<double> deviceTwdir() const { return _deviceTwdir; }
  void setDeviceTwdir(Angle<double> p) { _deviceTwdir = p; _flags |= DEVICE_TWDIR; }

  bool hasDeviceTwa() const { return _flags & DEVICE_TWA; }
  Angle<double> deviceTwa() const { return _deviceTwa; }
  void setDeviceTwa(Angle<double> p) { _deviceTwa = p; _flags |= DEVICE_TWA; }

 private:
  enum {
    DEVICE_SCREEN = 1,
    DEVICE_TARGET_VMG = 2,
    DEVICE_VMG = 4,
    DEVICE_TWDIR = 8,
    DEVICE_TWA = 16,
    DEVICE_TWS = 32
  };

  // contains entried from the enum above "ored" together.
  long _flags;

  Id _boatId;

  Velocity<double> _gpsSpeed;
  Angle<double> _awa;
  Velocity<double> _aws;

  // True wind angle and true wind speed as computed by thired-party onboard
  // instruments.
  Angle<double> _externalTwa;
  Velocity<double> _externalTws;

  Angle<double> _magHdg;
  Velocity<double> _watSpeed;
  Angle<double> _gpsBearing;

  GeographicPosition<double> _pos;

  // See NmeaParser: "Cumulative Water Distance" and "Water Distance"
  double _cwd;
  double _wd;

  // TIME RELATED
  TimeStamp _time;

  HorizontalMotion<double> _trueWind;
  ScreenInfo _deviceScreen;

  // Recorded information.
  Velocity<double> _deviceVmg;
  Velocity<double> _deviceTargetVmg;
  Velocity<double> _deviceTws;
  Angle<double> _deviceTwa;
  Angle<double> _deviceTwdir;
};


Array<Velocity<double> > getExternalTws(Array<Nav> navs);
Array<Angle<double> > getExternalTwa(Array<Nav> navs);
Array<Velocity<double> > getGpsSpeed(Array<Nav> navs);
Array<Angle<double> > getGpsBearing(Array<Nav> navs);
Array<Velocity<double> > getWatSpeed(Array<Nav> navs);
Array<Angle<double> > getMagHdg(Array<Nav> navs);
Array<Velocity<double> > getAws(Array<Nav> navs);
Array<Angle<double> > getAwa(Array<Nav> navs);

Array<Nav> loadNavsFromText(std::string filename, bool sort = true);
bool areSortedNavs(Array<Nav> navs);
void plotNavTimeVsIndex(Array<Nav> navs);
void dispNavTimeIntervals(Array<Nav> navs);
Array<Array<Nav> > splitNavsByDuration(Array<Nav> navs, Duration<double> dur);
MDArray2d calcNavsEcefTrajectory(Array<Nav> navs);
Array<MDArray2d> calcNavsEcefTrajectories(Array<Array<Nav> > navs);
void plotNavsEcefTrajectory(Array<Nav> navs);
void plotNavsEcefTrajectories(Array<Array<Nav> > navs);
int countNavs(Array<Array<Nav> > navs);

std::ostream &operator<<(std::ostream &s, const Nav &x);

Length<double> computeTrajectoryLength(Array<Nav> navs);
int findMaxSpeedOverGround(Array<Nav> navs);

} /* namespace sail */

#endif /* NAV_H_ */
