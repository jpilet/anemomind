/*
 * Nav.cpp
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#include "Nav.h"
#include <server/common/ArrayIO.h>
#include <algorithm>
#include <server/plot/gnuplot_i.hpp>
#include <server/common/LineKM.h>
#include <server/common/PhysicalQuantity.h>
#include <server/plot/extra.h>
#include <server/nautical/Ecef.h>
#include <ctime>
#include <server/nautical/WGS84.h>
#include <server/common/string.h>
#include <server/common/ArrayMapper.h>

namespace sail {

Nav::Nav() : _time(TimeStamp::makeUndefined()) {
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
  assert(row.rows() == 1);
  assert(row.cols() == 23 || row.cols() == 22);

  _gpsSpeed = Velocity<double>::knots(row(0, 6));
  _awa = Angle<double>::degrees(row(0, 7));
  _aws = Velocity<double>::knots(row(0, 8));

  _twaFromFile = Angle<double>::degrees(row(0, 9));
  _twsFromFile = Velocity<double>::knots(row(0, 10));

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
  return _gpsSpeed.eqWithNan(other._gpsSpeed) &&
      _awa.eqWithNan(other._awa) &&
      _aws.eqWithNan(other._aws) &&
      _boatId == other._boatId &&
      _twaFromFile.eqWithNan(other._twaFromFile) &&
      _twsFromFile.eqWithNan(other._twsFromFile) &&
      _magHdg.eqWithNan(other._magHdg) &&
      _watSpeed.eqWithNan(other._watSpeed) &&
      _gpsBearing.eqWithNan(other._gpsBearing) &&
      _pos == other._pos && (strictEquality(_cwd, other._cwd)) && (strictEquality(_wd, other._wd));
}

HorizontalMotion<double> Nav::apparentWind() const {
/* Important note: awa() is the angle w.r.t. the cource of the boat!
 * So awa() = 0 always means the boat is in irons */
  return HorizontalMotion<double>::polar(aws(), awa() + gpsBearing());
}
HorizontalMotion<double> Nav::gpsVelocity() const {
  return HorizontalMotion<double>::polar(gpsSpeed(), gpsBearing());
}

Angle<double> Nav::estimateRawTwa() const {
  return estimateRawTrueWind().angle() - gpsBearing();
}

Velocity<double> Nav::estimateRawTws() const {
  return estimateRawTrueWind().norm();
}

HorizontalMotion<double> Nav::estimateRawTrueWind() const {
  // Apparent = True - BoatVel <=> True = Apparent + BoatVel
  // E.g. if we are sailing downwind, the apparent wind will be close to zero and
  // the true wind will be nearly the same as the boat velocity.
  // If we are sailing upwind, the true wind and boat vel will point in opposite directions and we will have a strong
  // apparent wind.
  return apparentWind() + gpsVelocity();
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


const char Nav::AllNavsPath[] = "../../../../datasets/allnavs.txt";

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




//Array<Velocity<double> > getTws(Array<Nav> navs);
MAKE_ARRAY_MAPPER(getTws, Nav, Velocity<double>, x.estimateRawTws());


Array<Nav> loadNavsFromText(std::string filename, bool sort) {
  MDArray2d data = loadMatrixText<double>(filename);
  int count = data.rows();

  std::vector<Nav> navs(count);
  for (int i = 0; i < count; i++) {
    navs[i] = Nav(data.sliceRow(i));
  }

  if (sort) {
    std::sort(navs.begin(), navs.end());
  }

  return Array<Nav>::referToVector(navs).dup();
}

bool areSortedNavs(Array<Nav> navs) {
  int count = navs.size();
  for (int i = 0; i < count-1; i++) {
    if (navs[i].time() > navs[i+1].time()) {
      return false;
    }
  }
  return true;
}

void plotNavTimeVsIndex(Array<Nav> navs) {
  Gnuplot plot;
  int count = navs.size();

  std::vector<double> X(count), Y(count);
  TimeStamp start = navs[0].time();
  for (int i = 0; i < count; i++) {
    X[i] = i;
    Y[i] = (navs[i].time() - start).days();
  }

  plot.set_style("lines");
  plot.plot_xy(X, Y);
  plot.set_xautoscale();
  plot.set_yautoscale();
  plot.set_xlabel("Index");
  plot.set_ylabel("Time");
  plot.showonscreen();
  sleepForever();
}

double getNavsMaxInterval(Array<Nav> navs) {
  int count = navs.size();
  double m = 0.0;
  for (int i = 0; i < count-1; i++) {
    m = std::max(m, (navs[i+1].time() - navs[i].time()).seconds());
  }
  return m;
}

void dispNavTimeIntervals(Array<Nav> navs) {
  assert(areSortedNavs(navs));
  double mintime = 0.0;
  double maxtime = (navs[navs.size()-1].time() - navs[0].time()).seconds();

  double m = getNavsMaxInterval(navs);
  std::cout << "Max interval (seconds): " << m << std::endl;
  std::cout << "Span (seconds): " << maxtime - mintime << std::endl;
  int navCount = navs.size();
  int binCount = 30;
  LineKM line(log(2.0), log(m+1), 1, binCount);

  Arrayi bins(binCount);
  bins.setTo(0);
  for (int i = 0; i < navCount-1; i++) {
    double span = (navs[i+1].time() - navs[i].time()).seconds();
    int index = std::max(0, int(floor(line(log(span)))));
    bins[index]++;
  }
  for (int i = 0; i < binCount; i++) {
    cout << "Bin " << i+1 << "/" << binCount << ": " << bins[i]
      << " intervals longer than the previous but shorter than "
      << Duration<double>::seconds(exp(line.inv(i+1))).str() << endl;
  }
}

int countNavSplitsByDuration(Array<Nav> navs, double durSeconds) {
  int count = navs.size();
  int counter = 0;
  for (int i = 0; i < count-1; i++) {
    if ((navs[i+1].time() - navs[i].time()).seconds() > durSeconds) {
      counter++;
    }
  }
  return counter;
}

Array<Array<Nav> > splitNavsByDuration(Array<Nav> navs, double durSeconds) {
  int count = 1 + countNavSplitsByDuration(navs, durSeconds);
  Array<Array<Nav> > dst(count);
  int navCount = navs.size();
  int from = 0;
  int counter = 0;
  for (int i = 0; i < navCount-1; i++) {
    if ((navs[i+1].time() - navs[i].time()).seconds() > durSeconds) {
      dst[counter] = navs.slice(from, i+1);
      counter++;
      from = i+1;
    }
  }
  dst.last() = navs.sliceFrom(from);
  assert(counter + 1 == count);
  return dst;
}

MDArray2d calcNavsEcefTrajectory(Array<Nav> navs) {
  int count = navs.size();
  MDArray2d data(count, 3);
  for (int i = 0; i < count; i++) {
    Nav &nav = navs[i];

    Length<double> xyz[3];
    WGS84<double>::toXYZ(nav.geographicPosition(), xyz);


    for (int j = 0; j < 3; j++) {
      data(i, i) = xyz[j].meters();
    }
  }
  return data;
}

Array<MDArray2d> calcNavsEcefTrajectories(Array<Array<Nav> > navs) {
  int count = navs.size();
  Array<MDArray2d> dst(count);
  for (int i = 0; i < count; i++) {
    dst[i] = calcNavsEcefTrajectory(navs[i]);
  }
  return dst;
}

void plotNavsEcefTrajectory(Array<Nav> navs) {
  assert(areSortedNavs(navs));

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(calcNavsEcefTrajectory(navs));
  //plot.cmd("set view equal xyz");
  plot.show();
}

void plotNavsEcefTrajectories(Array<Array<Nav> > navs) {
  int count = navs.size();
  LineKM hue(0, count, 0.0, 360.0);

  GnuplotExtra plot;
  plot.set_style("lines");
  for (int i = 0; i < count; i++) {
    plot.plot(calcNavsEcefTrajectory(navs[i]));
  }
  plot.cmd("set view equal xyz");
  plot.show();
}

int countNavs(Array<Array<Nav> > navs) {
  int counter = 0;
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    counter += navs[i].size();
  }
  return counter;
}


} /* namespace sail */
