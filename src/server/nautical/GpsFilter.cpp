/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/GpsFilter.h>
#include <server/nautical/GeographicReference.h>
#include <server/common/Span.h>
#include <server/math/nonlinear/BandedSolver.h>
#include <algorithm>


namespace sail {
namespace GpsFilter {

Settings::Settings() :
    samplingPeriod(Duration<double>::seconds(1.0)) {
  filterSettings.iters = 4;
  filterSettings.regOrder = 2;
}

Duration<double> getLocalTime(TimeStamp timeRef, const Nav &nav) {
  return nav.time() - timeRef;
}

TimeStamp getGlobalTime(TimeStamp timeRef, Duration<double> localTime) {
  return timeRef + localTime;
}

Duration<double> getLocalTimeDif(const Array<Nav> &navs, int index) {
  auto li = navs.lastIndex();
  if (index == 0) {
    return (navs[1].time() - navs[0].time());
  } else if (index == li) {
    return (navs[li].time() - navs[li-1].time());
  } else {
    return std::min(navs[index+1].time() - navs[index].time(),
                        navs[index].time() - navs[index-1].time());
  }
}


int navIndexToTimeIndex(int navIndex) {
  return 1 + 2*navIndex;
}

const Nav &middle(Array<Nav> navs) {
  return navs[navs.middle()];
}

TimeStamp getTimeReference(Array<Nav> navs) {
  return middle(navs).time();
}

GeographicReference getGeographicReference(Array<Nav> navs) {
  return GeographicReference(middle(navs).geographicPosition());
}

GeographicReference::ProjectedPosition integrate(
    const HorizontalMotion<double> motion, Duration<double> dur) {
  auto x = Length<double>::meters(motion[0].metersPerSecond()*dur.seconds());
  auto y = Length<double>::meters(motion[1].metersPerSecond()*dur.seconds());
  return GeographicReference::ProjectedPosition{x, y};
}



Array<Observation<2> > getObservations(TimeStamp timeRef,
    GeographicReference geoRef, Array<Nav> navs, Sampling sampling) {
  int n = navs.size();
  Array<Observation<2> > dst(2*n);
  for (int i = 0; i < n; i++) {
    auto nav = navs[i];
    auto localTime = getLocalTime(timeRef, nav);
    auto localTimeDif = getLocalTimeDif(navs, i);
    auto difScale = localTimeDif.seconds()/sampling.period();
    auto weights = sampling.represent(localTime.seconds());
    auto geoDif = integrate(nav.gpsMotion(), localTimeDif);
    int offset = 2*i;
    auto localPos = geoRef.map(nav.geographicPosition());
    auto difWeights = weights;
    difWeights.lowerWeight = -difScale;
    difWeights.upperWeight = difScale;

    // Based on the position
    dst[offset + 0] = Observation<2>{weights,
      {localPos[0].meters(), localPos[1].meters()}};

    // Based on the speed
    dst[offset + 1] = Observation<2>{difWeights,
      {geoDif[0].meters(), geoDif[1].meters()}};
  }
  return dst;
}



Results filter(Array<Nav> navs, Settings settings) {
  assert(std::is_sorted(navs.begin(), navs.end()));
  auto timeRef = getTimeReference(navs);
  auto geoRef = getGeographicReference(navs);

  auto marg = Duration<double>::seconds(0.5);
  auto fromTime = getLocalTime(timeRef, navs.first()) - marg;
  auto toTime = getLocalTime(timeRef, navs.last()) + marg;
  int sampleCount = 2 + int(floor((toTime - fromTime)/settings.samplingPeriod));
  Sampling sampling(sampleCount, fromTime.seconds(), toTime.seconds());
  auto observations = getObservations(timeRef, geoRef, navs, sampling);
  MDArray2d X = BandedSolver::solve(AbsCost(), AbsCost(), sampling,
      observations, settings.filterSettings);
  return Results{navs, observations, sampling, X, timeRef, geoRef};
}

Array<Nav> Results::filteredNavs() const {

}

}
}
