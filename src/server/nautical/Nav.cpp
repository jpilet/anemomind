/*
 * Nav.cpp
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#include "Nav.h"
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/ArrayIO.h>
#include <algorithm>
#include <server/common/Span.h>
#include <server/plot/gnuplot_i.hpp>
#include <server/common/LineKM.h>
#include <server/plot/extra.h>
#include <server/nautical/Ecef.h>
#include <ctime>
#include <server/nautical/WGS84.h>
#include <server/common/string.h>
#include <server/common/PhysicalQuantityIO.h>
#include <server/common/logging.h>
#include <server/common/Functional.h>
#include <device/anemobox/Dispatcher.h>


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


namespace {
  template <typename T>
  bool isDefined(const T &x) {
    return !x.isNaN();
  }

  template <>
  bool isDefined<GeographicPosition<double> >(const GeographicPosition<double> &x) {
    return true;
  }

  template <typename T>
  typename sail::TimedSampleCollection<T>::TimedVector getTimedVectorFromNavs(
      const Array<Nav> &navs, std::function<T(const Nav &)> f) {
    typename sail::TimedSampleCollection<T>::TimedVector dst;
    for (const auto &x: navs) {
      if (x.time().defined()) {
        auto v = f(x);
        if (isDefined(v)) {
          dst.push_back(TimedValue<T>(x.time(), v));
        }
      }
    }
    return dst;
  }

  std::shared_ptr<Dispatcher> makeDispatcherFromNavs(const Array<Nav> &navs) {
    const char srcOurs[] = "NavOurs";
    const char srcExternal[] = "NavExternal";

    auto dst = std::make_shared<Dispatcher>();

    dst->insertValues<Angle<double> >(
      AWA, srcOurs, getTimedVectorFromNavs<Angle<double> >(navs, [](const Nav &x) {
      return x.awa();
    }));

    dst->insertValues<Velocity<double> >(
      AWS, srcOurs, getTimedVectorFromNavs<Velocity<double> >(navs, [](const Nav &x) {
      return x.aws();
    }));

    dst->insertValues<Angle<double> >(
      TWA, srcExternal, getTimedVectorFromNavs<Angle<double> >(navs, [](const Nav &x) {
      return x.externalTwa();
    }));

    dst->insertValues<Velocity<double> >(
      TWS, srcExternal, getTimedVectorFromNavs<Velocity<double> >(navs, [](const Nav &x) {
      return x.externalTws();
    }));

    dst->insertValues<Angle<double> >(
      TWDIR, srcExternal, getTimedVectorFromNavs<Angle<double> >(navs, [](const Nav &x) {
      return x.externalTwdir();
    }));

    dst->insertValues<Angle<double> >(
      TWA, srcOurs, getTimedVectorFromNavs<Angle<double> >(navs, [](const Nav &x) {
      return x.deviceTwa();
    }));

    dst->insertValues<Velocity<double> >(
      TWS, srcOurs, getTimedVectorFromNavs<Velocity<double> >(navs, [](const Nav &x) {
      return x.deviceTws();
    }));

    dst->insertValues<Angle<double> >(
      TWDIR, srcOurs, getTimedVectorFromNavs<Angle<double> >(navs, [](const Nav &x) {
      return x.deviceTwdir();
    }));

    dst->insertValues<Velocity<double> >(
      GPS_SPEED, srcOurs, getTimedVectorFromNavs<Velocity<double> >(navs, [](const Nav &x) {
      return x.gpsSpeed();
    }));

    dst->insertValues<Angle<double> >(
      GPS_BEARING, srcOurs, getTimedVectorFromNavs<Angle<double> >(navs, [](const Nav &x) {
      return x.gpsBearing();
    }));

    dst->insertValues<Angle<double> >(
      MAG_HEADING, srcOurs, getTimedVectorFromNavs<Angle<double> >(navs, [](const Nav &x) {
      return x.magHdg();
    }));

    dst->insertValues<Velocity<double> >(
      WAT_SPEED, srcOurs, getTimedVectorFromNavs<Velocity<double> >(navs, [](const Nav &x) {
      return x.watSpeed();
    }));

    dst->insertValues<GeographicPosition<double> >(
      GPS_POS, srcOurs, getTimedVectorFromNavs<GeographicPosition<double> >(navs, [](const Nav &x) {
      return x.geographicPosition();
    }));

    dst->insertValues<Velocity<double> >(
      TARGET_VMG, srcOurs, getTimedVectorFromNavs<Velocity<double> >(navs, [](const Nav &x) {
      return x.deviceTargetVmg();
    }));

    dst->insertValues<Velocity<double> >(
      VMG, srcOurs, getTimedVectorFromNavs<Velocity<double> >(navs, [](const Nav &x) {
      return x.deviceVmg();
    }));

    return dst;
  }

}

NavCollection::NavCollection(const std::shared_ptr<Dispatcher> &dispatcher) :
  _dispatcher(dispatcher) {}

int NavCollection::size() const {
  return bool(_dispatcher)? _dispatcher->getNonEmptyValues<GPS_POS>().size() : 0;
}

namespace {


  class SummaryVisitor {
   public:
    SummaryVisitor(std::ostream *out) : _out(out) {}

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
        const TimedSampleCollection<T> &coll) {
      *_out << "  Channel of type " << shortName << " named " << sourceName << " with " << coll.size() << " samples\n";
    }
   private:
    std::ostream *_out;
  };

  template <typename Mapper>
  void visitChannels(const std::shared_ptr<Dispatcher> &dispatcher, Mapper m) {
    for (const auto &codeAndSources: dispatcher->allSources()) {
      auto c = codeAndSources.first;
      for (const auto &kv: codeAndSources.second) {

#define TRY_TO_MAP(handle, code, shortname, type, description) \
    if (c == handle) {\
      m.template visit<handle, type >(shortname, kv.first, \
          toTypedDispatchData<handle>(kv.second)->dispatcher()->values());\
    }\

    FOREACH_CHANNEL(TRY_TO_MAP)

#undef TRY_TO_MAP

      }
    }
  }

}


void NavCollection::outputSummary(std::ostream *dst) const {
  //visitChannels<SummaryVisitor>(_dispatcher, SummaryVisitor(dst));
  visitChannels(_dispatcher, SummaryVisitor(dst));
}

bool NavCollection::isValidNavIndex(int i) const {
  return 0 <= i && i < size();
}

bool NavCollection::isValidNavBoundaryIndex(int i) const {
  return 0 <= i && i <= size();
}

namespace {
  const auto maxGapSeconds = 12;

  template <DataCode Code>
  Optional<typename TypeForCode<Code>::type> getValue(
      const std::shared_ptr<Dispatcher> &src, const TimeStamp &time) {
    auto nearest = src->getNonEmptyValues<Code>().nearestTimedValue(time);
    if (nearest.defined()) {
      if (abs((nearest.get().time - time).seconds()) < maxGapSeconds) {
        return Optional<typename TypeForCode<Code>::type>(nearest.get().value);
      }
    }
    return Optional<typename TypeForCode<Code>::type>();
  }
}

#define SET_NAV_VALUE_F(DATACODE, SET) \
  {auto x = getValue<DATACODE>(_dispatcher, timeAndPos.time); if (x.defined()) {SET}}

#define SET_NAV_VALUE(DATACODE, METHOD) SET_NAV_VALUE_F(DATACODE, dst.METHOD(x());)

const Nav NavCollection::operator[] (int i) const {
  assert(isValidNavIndex(i));
  const auto &samples = _dispatcher->getNonEmptyValues<GPS_POS>().samples();
  auto timeAndPos = samples[i];

  Nav dst;
  dst.setBoatId(Nav::debuggingBoatId());
  dst.setTime(timeAndPos.time);
  dst.setGeographicPosition(timeAndPos.value);
  SET_NAV_VALUE(AWA, setAwa);
  SET_NAV_VALUE(AWS, setAws);

  // Well, it is not very clear how
  // the different fields in the Nav map
  // to different sources. Hope we
  // can get rid of Nav altogether
  // in the not so distant future.
  SET_NAV_VALUE(TWA, setDeviceTwa);
  SET_NAV_VALUE(TWA, setExternalTwa);
  SET_NAV_VALUE(TWS, setDeviceTws);
  SET_NAV_VALUE(TWS, setExternalTws);

  SET_NAV_VALUE(GPS_SPEED, setGpsSpeed);
  SET_NAV_VALUE(GPS_BEARING, setGpsBearing);
  SET_NAV_VALUE(MAG_HEADING, setMagHdg);
  SET_NAV_VALUE(WAT_SPEED, setWatSpeed);
  SET_NAV_VALUE(TARGET_VMG, setDeviceTargetVmg);
  SET_NAV_VALUE(VMG, setDeviceVmg);
  return dst;
}

namespace {
  class TimeSlicer {
   public:
    TimeSlicer(const TimeStamp &from, const TimeStamp &to) : _from(from), _to(to) {}

    template <DataCode Code, typename T>
    typename TimedSampleCollection<T>::TimedVector map(const TimedSampleCollection<T> &coll) {
      typename TimedSampleCollection<T>::TimedVector dst;
      for (auto x: coll.samples()) {
        if (_from <= x.time && x.time <= _to) {
          dst.push_back(x);
        }
      }
      return dst;
    }
   private:
    TimeStamp _from, _to;
  };

  template <typename Mapper>
  std::shared_ptr<Dispatcher> mapDispatcherChannels(const std::shared_ptr<Dispatcher> &dispatcher, Mapper m) {
    auto dst = std::make_shared<Dispatcher>();

    // Setting the priorities should be done first!
    for (const auto &codeAndSources: dispatcher->allSources()) {
      auto c = codeAndSources.first;
      for (const auto &kv: codeAndSources.second) {
        dst->setSourcePriority(kv.first, dispatcher->sourcePriority(kv.first));
      }
    }

    for (const auto &codeAndSources: dispatcher->allSources()) {
      auto c = codeAndSources.first;
      for (const auto &kv: codeAndSources.second) {

#define TRY_TO_MAP(handle, code, description, type, shortname) \
    if (c == handle) {\
      auto y = m.template map<handle, type >(\
          toTypedDispatchData<handle>(kv.second)->dispatcher()->values());\
      dst->insertValues<type >(handle, kv.first, y);\
    }\

    FOREACH_CHANNEL(TRY_TO_MAP)

#undef TRY_TO_MAP

      }
    }
    return dst;
  }

}



NavCollection NavCollection::slice(int from, int to) const {
  assert(isValidNavBoundaryIndex(from));
  assert(isValidNavBoundaryIndex(to));
  assert(from <= to);
  if (to == 0) {
    return NavCollection();
  }
  auto samples = _dispatcher->getNonEmptyValues<GPS_POS>().samples();
  auto leftTime = samples[from].time; // inclusive
  auto rightTime = samples[to-1].time; // inclusive (because we take the time one index before).
  auto f = [&](const TimeStamp &t) {
    return leftTime <= t && t <= rightTime;
  };

  return NavCollection(mapDispatcherChannels(_dispatcher, TimeSlicer(leftTime, rightTime)));
}

NavCollection NavCollection::sliceFrom(int index) const {
  return slice(index, size());
}

NavCollection NavCollection::sliceTo(int index) const {
  return slice(0, index);
}

const Nav NavCollection::last() const {
  return (*this)[lastIndex()];
}

const Nav NavCollection::first() const {
  return (*this)[0];
}

/*ArrayIterator<NavCollection> NavCollection::begin() const {
  return ArrayIterator<NavCollection>(*this, 0);
}

ArrayIterator<NavCollection> NavCollection::end() const {
  return ArrayIterator<NavCollection>(*this, size());
}*/

int NavCollection::middle() const {
  return size()/2;
}

int NavCollection::lastIndex() const {
  return size() - 1;
}

bool NavCollection::empty() const {
  return size() == 0;
}

Array<Nav> NavCollection::makeArray() const {
  int n = size();
  Array<Nav> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = (*this)[i];
  }
  return dst;
}

NavCollection NavCollection::fromNavs(const Array<Nav> &navs) {
  return NavCollection(makeDispatcherFromNavs(navs));
}
/*:
    _dispatcher() {}*/

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




NavCollection loadNavsFromText(std::string filename, bool sort) {
  MDArray2d data = loadMatrixText<double>(filename);
  int count = data.rows();

  std::vector<Nav> navs(count);
  for (int i = 0; i < count; i++) {
    navs[i] = Nav(data.sliceRow(i));
  }

  if (sort) {
    std::sort(navs.begin(), navs.end());
  }

  return NavCollection::fromNavs(Array<Nav>::referToVector(navs).dup());
}

bool areSortedNavs(NavCollection navs) {
  int count = navs.size();
  for (int i = 0; i < count-1; i++) {
    if (navs[i].time() > navs[i+1].time()) {
      return false;
    }
  }
  return true;
}

void plotNavTimeVsIndex(NavCollection navs) {
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

double getNavsMaxInterval(NavCollection navs) {
  int count = navs.size();
  double m = 0.0;
  for (int i = 0; i < count-1; i++) {
    m = std::max(m, (navs[i+1].time() - navs[i].time()).seconds());
  }
  return m;
}

void dispNavTimeIntervals(NavCollection navs) {
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

int countNavSplitsByDuration(NavCollection navs, Duration<double> dur) {
  int count = navs.size();
  int counter = 0;
  for (int i = 0; i < count-1; i++) {
    if ((navs[i+1].time() - navs[i].time()) > dur) {
      counter++;
    }
  }
  return counter;
}



Array<NavCollection > splitNavsByDuration(NavCollection navs, Duration<double> dur) {
  int count = 1 + countNavSplitsByDuration(navs, dur);
  Array<NavCollection> dst(count);
  int navCount = navs.size();
  int from = 0;
  int counter = 0;
  for (int i = 0; i < navCount-1; i++) {
    if ((navs[i+1].time() - navs[i].time()) > dur) {
      dst[counter] = navs.slice(from, i+1);
      counter++;
      from = i+1;
    }
  }
  dst.last() = navs.sliceFrom(from);
  assert(counter + 1 == count);
  return dst;
}

/*Array<NavCollection > splitNavsByDuration(NavCollection navs, double durSeconds) {
  return splitNavsByDuration(navs, Duration<double>::seconds(durSeconds));
}*/

MDArray2d calcNavsEcefTrajectory(NavCollection navs) {
  int count = navs.size();
  MDArray2d data(count, 3);
  for (int i = 0; i < count; i++) {
    Nav nav = navs[i];

    Length<double> xyz[3];
    WGS84<double>::toXYZ(nav.geographicPosition(), xyz);


    for (int j = 0; j < 3; j++) {
      data(i, i) = xyz[j].meters();
    }
  }
  return data;
}

Array<MDArray2d> calcNavsEcefTrajectories(Array<NavCollection > navs) {
  int count = navs.size();
  Array<MDArray2d> dst(count);
  for (int i = 0; i < count; i++) {
    dst[i] = calcNavsEcefTrajectory(navs[i]);
  }
  return dst;
}

void plotNavsEcefTrajectory(NavCollection navs) {
  assert(areSortedNavs(navs));

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(calcNavsEcefTrajectory(navs));
  //plot.cmd("set view equal xyz");
  plot.show();
}

void plotNavsEcefTrajectories(Array<NavCollection > navs) {
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

int countNavs(Array<NavCollection > navs) {
  int counter = 0;
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    counter += navs[i].size();
  }
  return counter;
}

std::ostream &operator<<(std::ostream &s, const Nav &x) {
  s << "Nav:\n";
  s << "  maghdg: " << x.magHdg() << "\n";
  s << "  aws: " << x.aws() << "\n";
  s << "  awa: " << x.awa() << "\n";
  s << "  watspeed: " << x.watSpeed() << "\n";
  s << "  gps bearing: " << x.gpsBearing() << "\n";
  s << "  gps speed: " << x.gpsSpeed() << "\n";
  return s;
}

Length<double> computeTrajectoryLength(NavCollection navs) {
  Length<double> dist = Length<double>::meters(0.0);
  int n = navs.size() - 1;
  for (int i = 0; i < n; i++) {
    dist = dist + distance(navs[i].geographicPosition(), navs[i+1].geographicPosition());
  }
  return dist;
}

int findMaxSpeedOverGround(NavCollection navs) {
  auto marg = Duration<double>::minutes(2.0);
  Span<TimeStamp> validTime(navs.first().time() + marg, navs.last().time() - marg);
  int bestIndex = -1;
  auto maxSOG = Velocity<double>::knots(-1.0);
  for (int i = 0; i < navs.size(); ++i) {
    const Nav &nav = navs[i];
    Velocity<double> sog = nav.gpsSpeed();
    if (!sog.isNaN() && maxSOG < sog && validTime.contains(nav.time())) {
      maxSOG = sog;
      bestIndex = i;
    }
  }
  return bestIndex;
}

} /* namespace sail */
