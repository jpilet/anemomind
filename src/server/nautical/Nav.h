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

namespace sail {


// Represents a single recording of data from the devices onboard.
class Nav {
 public:
  Nav();
  Nav(MDArray2d row);
  virtual ~Nav();

  // For sorting
  bool operator< (const Nav &other) const {
    return _timeDays < other._timeDays;
  }

  double getTimeDays() const {
    return _timeDays;
  }
  double getTimeSeconds() const {
    return 24*60*60*_timeDays;
  }

  double getLonRadians() const;
  double getLatRadians() const;

  void getEcef3dPos(double &xOut, double &yOut, double &zOut) const;

  // This is just temporary. We should
  // replace it with CMake-generated paths in the future.
  static const char AllNavsPath[];
 private:
  double _year;
  double _month;
  double _dayOfTheMonth;
  double _hour;
  double _minute;
  double _second;
  double _gpsSpeed;
  double _awa;
  double _aws;

  // Can we trust these estimates of the true wind? Don't think so. We'd better reconstruct them
  // with a good model.
  double _twaFromFile;
  double _twsFromFile;

  double _magHdg;
  double _watSpeed;
  double _gpsBearing;
  double _posLatDeg;
  double _posLatMin;
  double _posLatMc;
  double _posLonDeg;
  double _posLonMin;
  double _posLonMc;
  double _cwd;
  double _wd;

  // Time in days
  double _timeDays;
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
