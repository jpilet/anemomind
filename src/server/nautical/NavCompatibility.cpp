/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/ArrayIO.h>
#include <server/common/Functional.h>
#include <server/common/LineKM.h>
#include <server/common/Span.h>
#include <server/nautical/NavCompatibility.h>
#include <server/nautical/WGS84.h>
#include <server/plot/extra.h>

namespace sail {

namespace {
  template <typename T>
  bool isDefined(const T &x) {
    return isFinite(x);
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

  void  insertNavsIntoDispatcher(const Array<Nav> &navs, Dispatcher *dst) {
    const char srcOurs[] = "NavDevice";
    const char srcExternal[] = "NavExternal";

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
  }

  std::shared_ptr<Dispatcher> makeDispatcherFromNavs(const Array<Nav> &navs) {
    auto dst = std::make_shared<Dispatcher>();
    insertNavsIntoDispatcher(navs, dst.get());
    return dst;
  }

}

namespace {
  TimedSampleRange<GeographicPosition<double> > getGpsPositions(
      const NavDataset &ds) {
    return ds.samples<GPS_POS>();
  }

  const auto maxGapSeconds = 12;

  template <DataCode Code>
  Optional<typename TypeForCode<Code>::type> getValue(
      const NavDataset &src, const TimeStamp &time) {
    auto nearest = src.samples<Code>().nearest(time);
    if (nearest.defined()) {
      if (abs((nearest.get().time - time).seconds()) < maxGapSeconds) {
        return Optional<typename TypeForCode<Code>::type>(nearest.get().value);
      }
    }
    return Optional<typename TypeForCode<Code>::type>();
  }

  template<DataCode code, class T>
  void setNavValue(const NavDataset &dataset, TimeStamp time, Nav *dst, void (Nav::* set)(T)) {
      auto x = getValue<code>(dataset, time);
      if (x.defined()) { ((*dst).*set)(x()); }
  }


  Array<std::string> orderedDeviceSources{"NavDevice"};
  Array<std::string> orderedExternalSources{"NavExternal"};

  std::map<std::string, int> devicePriorities{
    {"NavDevice", 2},
    {"NavExternal", 1}
  };


  template <DataCode Code>
  Optional<typename TypeForCode<Code>::type> lookUpPrioritizedSample(
      const std::shared_ptr<Dispatcher> &dispatcher,
      TimeStamp time, const Array<std::string> &orderedSources) {
    typedef typename TypeForCode<Code>::type T;
    const auto &all = dispatcher->allSources();
    auto found = all.find(Code);
    if (found != all.end()) {
      const auto &sources = found->second;
      for (auto srcName: orderedSources) {
        auto found = sources.find(srcName);
        if (found != sources.end()) {
          const TimedSampleCollection<T> &data = toTypedDispatchData<Code>(found->second.get())->dispatcher()->values();
          auto nearest = findNearestTimedValue<T>(data.samples().begin(), data.samples().end(), time);
          if (nearest.defined()) {
            if (std::abs((nearest.get().time - time).seconds()) < maxMergeDifSeconds) {
              return Optional<T>(nearest.get().value);
            }
          }
        }
      }
    }
    return Optional<T>();
  }

  template <DataCode code, class T>
  void setNavValueMultiSource(
      const Array<std::string> &orderedSources,
      const NavDataset &data, TimeStamp time, Nav *dst, void (Nav::* set)(T)) {
    auto sample = lookUpPrioritizedSample<code>(data.dispatcher(), time, orderedSources);
    if (sample.defined()) {
      ((*dst).*set)(sample.get());
    } else {
      setNavValue<code, T>(data, time, dst, set);
    }
  }

}


namespace NavCompat {

int getNavSize(const NavDataset &ds) {
  return getGpsPositions(ds).size();
}



bool isValidNavIndex(const NavDataset &ds, int i) {
  return 0 <= i && i < getNavSize(ds);
}

bool isValidNavBoundaryIndex(const NavDataset &ds, int i) {
  return 0 <= i && i <= getNavSize(ds);
}

const Nav getNav(const NavDataset &ds, int i) {
  assert(isValidNavIndex(ds, i));
  const auto &samples = getGpsPositions(ds);
  auto timeAndPos = samples[i];

  Nav dst;
  dst.setBoatId(Nav::debuggingBoatId());
  dst.setTime(timeAndPos.time);
  dst.setGeographicPosition(timeAndPos.value);


  setNavValue<AWA>(ds, timeAndPos.time, &dst, &Nav::setAwa);
  setNavValue<AWS>(ds, timeAndPos.time, &dst, &Nav::setAws);

  setNavValueMultiSource<TWA>(orderedDeviceSources, ds, timeAndPos.time, &dst, &Nav::setDeviceTwa);
  setNavValueMultiSource<TWA>(orderedExternalSources, ds, timeAndPos.time, &dst, &Nav::setExternalTwa);
  setNavValueMultiSource<TWS>(orderedDeviceSources, ds, timeAndPos.time, &dst, &Nav::setDeviceTws);
  setNavValueMultiSource<TWS>(orderedExternalSources, ds, timeAndPos.time, &dst, &Nav::setExternalTws);

  setNavValue<GPS_SPEED>(ds, timeAndPos.time, &dst, &Nav::setGpsSpeed);
  setNavValue<GPS_BEARING>(ds, timeAndPos.time, &dst, &Nav::setGpsBearing);
  setNavValue<MAG_HEADING>(ds, timeAndPos.time, &dst, &Nav::setMagHdg);
  setNavValue<WAT_SPEED>(ds, timeAndPos.time, &dst, &Nav::setWatSpeed);
  setNavValue<VMG>(ds, timeAndPos.time, &dst, &Nav::setDeviceVmg);

  return dst;
}

NavDataset slice(const NavDataset &ds, int from, int to) {
  assert(isValidNavBoundaryIndex(ds, from));
  assert(isValidNavBoundaryIndex(ds, to));
  assert(from <= to);
  if (to == 0) {
    return NavDataset();
  }
  auto samples = getGpsPositions(ds);
  auto leftTime = samples[from].time; // inclusive
  auto rightTime = samples[to-1].time; // inclusive (because we take the time one index before).

  return ds.slice(leftTime, rightTime);
}

NavDataset sliceFrom(const NavDataset &ds, int index) {
  return slice(ds, index, getNavSize(ds));
}

NavDataset sliceTo(const NavDataset &ds, int index) {
  return slice(ds, 0, index);
}

const Nav getLast(const NavDataset &ds) {
  return getNav(ds, getLastIndex(ds));
}

const Nav getFirst(const NavDataset &ds) {
  return getNav(ds, 0);
}

int getMiddleIndex(const NavDataset &ds) {
  return getNavSize(ds)/2;
}

int getLastIndex(const NavDataset &ds) {
  return getNavSize(ds) - 1;
}

bool isEmpty(const NavDataset &ds) {
  return getNavSize(ds) == 0;
}

Array<Nav> makeArray(const NavDataset &ds) {
  int n = getNavSize(ds);
  Array<Nav> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = getNav(ds, i);
  }
  return dst;
}

NavDataset fromNavs(const Array<Nav> &navs) {
  return NavDataset(makeDispatcherFromNavs(navs));
}

Iterator getBegin(const NavDataset &ds) {
  return Iterator(ds, 0);
}

Iterator getEnd(const NavDataset &ds) {
  return Iterator(ds, getNavSize(ds));
}

}

using namespace NavCompat;

Array<MDArray2d> calcNavsEcefTrajectories(Array<NavDataset > navs) {
  int count = navs.size();
  Array<MDArray2d> dst(count);
  for (int i = 0; i < count; i++) {
    dst[i] = calcNavsEcefTrajectory(navs[i]);
  }
  return dst;
}

void plotNavsEcefTrajectory(NavDataset navs) {
  assert(areSortedNavs(navs));

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(calcNavsEcefTrajectory(navs));
  //plot.cmd("set view equal xyz");
  plot.show();
}

void plotNavsEcefTrajectories(Array<NavDataset > navs) {
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

int countNavs(Array<NavDataset > navs) {
  int counter = 0;
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    counter += NavCompat::getNavSize(navs[i]);
  }
  return counter;
}

NavDataset loadNavsFromText(std::string filename, bool sort) {
  MDArray2d data = loadMatrixText<double>(filename);
  int count = data.rows();

  std::vector<Nav> navs(count);
  for (int i = 0; i < count; i++) {
    navs[i] = Nav(data.sliceRow(i));
  }

  if (sort) {
    std::sort(navs.begin(), navs.end());
  }

  return NavCompat::fromNavs(Array<Nav>::referToVector(navs).dup());
}

bool areSortedNavs(NavDataset navs) {
  int count = NavCompat::getNavSize(navs);
  for (int i = 0; i < count-1; i++) {
    if (NavCompat::getNav(navs, i).time() > NavCompat::getNav(navs, i+1).time()) {
      return false;
    }
  }
  return true;
}

void plotNavTimeVsIndex(NavDataset navs) {
  Gnuplot plot;
  int count = NavCompat::getNavSize(navs);

  std::vector<double> X(count), Y(count);
  TimeStamp start = NavCompat::getNav(navs, 0).time();
  for (int i = 0; i < count; i++) {
    X[i] = i;
    Y[i] = (NavCompat::getNav(navs, i).time() - start).days();
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

double getNavsMaxInterval(NavDataset navs) {
  int count = NavCompat::getNavSize(navs);
  double m = 0.0;
  for (int i = 0; i < count-1; i++) {
    m = std::max(m, (NavCompat::getNav(navs, i+1).time() - NavCompat::getNav(navs, i).time()).seconds());
  }
  return m;
}

void dispNavTimeIntervals(NavDataset navs) {
  assert(areSortedNavs(navs));
  double mintime = 0.0;
  double maxtime = (getNav(navs, getNavSize(navs)-1).time() - getNav(navs, 0).time()).seconds();

  double m = getNavsMaxInterval(navs);
  std::cout << "Max interval (seconds): " << m << std::endl;
  std::cout << "Span (seconds): " << maxtime - mintime << std::endl;
  int navCount = getNavSize(navs);
  int binCount = 30;
  LineKM line(log(2.0), log(m+1), 1, binCount);

  Arrayi bins(binCount);
  bins.setTo(0);
  for (int i = 0; i < navCount-1; i++) {
    double span = (getNav(navs, i+1).time() - getNav(navs, i).time()).seconds();
    int index = std::max(0, int(floor(line(log(span)))));
    bins[index]++;
  }
  for (int i = 0; i < binCount; i++) {
    cout << "Bin " << i+1 << "/" << binCount << ": " << bins[i]
      << " intervals longer than the previous but shorter than "
      << Duration<double>::seconds(exp(line.inv(i+1))).str() << endl;
  }
}

int countNavSplitsByDuration(NavDataset navs, Duration<double> dur) {
  int count = getNavSize(navs);
  int counter = 0;
  for (int i = 0; i < count-1; i++) {
    if ((getNav(navs, i+1).time() - getNav(navs, i).time()) > dur) {
      counter++;
    }
  }
  return counter;
}



Array<NavDataset > splitNavsByDuration(NavDataset navs, Duration<double> dur) {
  int count = 1 + countNavSplitsByDuration(navs, dur);
  Array<NavDataset> dst(count);
  int navCount = getNavSize(navs);
  int from = 0;
  int counter = 0;
  for (int i = 0; i < navCount-1; i++) {
    if ((getNav(navs, i+1).time() - getNav(navs, i).time()) > dur) {
      dst[counter] = slice(navs, from, i+1);
      counter++;
      from = i+1;
    }
  }
  dst.last() = sliceFrom(navs, from);
  assert(counter + 1 == count);
  return dst;
}

/*Array<NavDataset > splitNavsByDuration(NavDataset navs, double durSeconds) {
  return splitNavsByDuration(navs, Duration<double>::seconds(durSeconds));
}*/

MDArray2d calcNavsEcefTrajectory(NavDataset navs) {
  int count = getNavSize(navs);
  MDArray2d data(count, 3);
  for (int i = 0; i < count; i++) {
    Nav nav = getNav(navs, i);

    Length<double> xyz[3];
    WGS84<double>::toXYZ(nav.geographicPosition(), xyz);

    for (int j = 0; j < 3; j++) {
      data(i, i) = xyz[j].meters();
    }
  }
  return data;
}

Length<double> computeTrajectoryLength(NavDataset navs) {
  Length<double> dist = Length<double>::meters(0.0);
  int n = getNavSize(navs) - 1;
  for (int i = 0; i < n; i++) {
    dist = dist + distance(getNav(navs, i).geographicPosition(), getNav(navs, i+1).geographicPosition());
  }
  return dist;
}

int findMaxSpeedOverGround(NavDataset navs) {
  auto marg = Duration<double>::minutes(2.0);
  Span<TimeStamp> validTime(getFirst(navs).time() + marg, getLast(navs).time() - marg);
  int bestIndex = -1;
  auto maxSOG = Velocity<double>::knots(-1.0);
  for (int i = 0; i < getNavSize(navs); ++i) {
    const Nav &nav = getNav(navs, i);
    Velocity<double> sog = nav.gpsSpeed();
    if (!isNaN(sog) && maxSOG < sog && validTime.contains(nav.time())) {
      maxSOG = sog;
      bestIndex = i;
    }
  }
  return bestIndex;
}

}
