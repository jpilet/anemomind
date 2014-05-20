/*
 * Nav.h
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#ifndef NAV_H_
#define NAV_H_

#include <string>
#include <server/common/MDArray.h>
#include <server/common/math.h>
#include <server/common/PhysicalQuantity.h>
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

  static Nav::Id debuggingBoatId() {return "FFFFFFFF";}


  Nav();
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
  Angle<double> awa() const {return _awa;}
  Velocity<double> aws() const {return _aws;}
  Angle<double> magHdg() const {return _magHdg;}
  Angle<double> gpsBearing() const {return _gpsBearing;}
  Velocity<double> gpsSpeed() const {return _gpsSpeed;}
  Velocity<double> watSpeed() const {return _watSpeed;}

  // Provide a raw estimate of the true wind,
  // based on simple vector addition, assuming
  // all instruments are perfectly calibrated.
  HorizontalMotion<double> estimateRawTrueWind() const;

  HorizontalMotion<double> apparentWind() const;
  HorizontalMotion<double> gpsVelocity() const;

  void setAwa(Angle<double> awa_) {_awa = awa_;}
  void setAws(Velocity<double> aws_) {_aws = aws_;}
  void setMagHdg(Angle<double> magHdg_) {_magHdg = magHdg_;}
  void setGpsBearing(Angle<double> gpsBearing_) {_gpsBearing = gpsBearing_;}
  void setGpsSpeed(Velocity<double> gpsSpeed_) {_gpsSpeed = gpsSpeed_;}
  void setWatSpeed(Velocity<double> watSpeed_) {_watSpeed = watSpeed_;}
  void setTime(const TimeStamp &t) {_time = t;}
  void setGeographicPosition(GeographicPosition<double> pos) {_pos = pos;}

  // This is just temporary. We should
  // replace it with CMake-generated paths in the future.
  static const char AllNavsPath[];

  bool operator== (const Nav &other) const;

  void setBoatId(const Id &bi) {_boatId = bi;}
  bool hasBoatId() const {return !_boatId.empty();}

  // TODO: Require this method to return true before a Nav is inserted to a database.
  bool isIndexed() const {return hasId() && hasBoatId();}

  Id id() const;
  bool hasId() const;
  const Id &boatId() const {return _boatId;}
 private:
  Id _boatId;

  Velocity<double> _gpsSpeed;
  Angle<double> _awa;
  Velocity<double> _aws;

  // Can we trust these estimates of the true wind? Don't think so. We'd better reconstruct them
  // with a good model.
  Angle<double> _twaFromFile;
  Velocity<double> _twsFromFile;

  Angle<double> _magHdg;
  Velocity<double> _watSpeed;
  Angle<double> _gpsBearing;

  GeographicPosition<double> _pos;

  // What does cwd and wd stand for? I forgot...
  // See NmeaParser: "Cumulative Water Distance" and "Water Distance"
  double _cwd;
  double _wd;



  // TIME RELATED
  TimeStamp _time;
};

Array<Nav> loadNavsFromText(std::string filename, bool sort = true);
bool areSortedNavs(Array<Nav> navs);
void plotNavTimeVsIndex(Array<Nav> navs);
void dispNavTimeIntervals(Array<Nav> navs);
Array<Array<Nav> > splitNavsByDuration(Array<Nav> navs, double durSeconds);
MDArray2d calcNavsEcefTrajectory(Array<Nav> navs);
Array<MDArray2d> calcNavsEcefTrajectories(Array<Array<Nav> > navs);
void plotNavsEcefTrajectory(Array<Nav> navs);
void plotNavsEcefTrajectories(Array<Array<Nav> > navs);
int countNavs(Array<Array<Nav> > navs);


} /* namespace sail */

#endif /* NAV_H_ */
