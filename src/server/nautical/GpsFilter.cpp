/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/GpsFilter.h>
#include <server/nautical/GeographicReference.h>
#include <server/common/ArrayBuilder.h>
#include <algorithm>


namespace sail {
namespace GpsFilter {

Settings::Settings() :
    samplingPeriod(Duration<double>::seconds(1.0))
    {}

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

struct GpsPoint {
  Sampling::Weights weights;
  double posMeters[2];
  double difMeters[2]; // How big the difference should be between the two samples
};

Array<GpsPoint> getPoints(TimeStamp timeRef,
    GeographicReference geoRef, Array<Nav> navs, Sampling sampling) {
  return Spani(0, navs.size()).map<GpsPoint>([&](const Nav &nav) {
    return GpsPoint();
  });
}

Array<Nav> filter(Array<Nav> navs, Settings settings) {
  assert(std::is_sorted(navs.begin(), navs.end()));
  auto timeRef = getTimeReference(navs);
  auto geoRef = getGeographicReference(navs);

  double fromTimeSeconds = getLocalTime(timeRef, navs.first()) - 0.5;
  double toTimeSeconds = getLocalTime(timeRef, navs.last()) + 0.5;
  int sampleCount = 2 + int(floor((toTimeSeconds - fromTimeSeconds)/settings.samplingPeriod.seconds()));
  Sampling sampling(sampleCount, fromTimeSeconds, toTimeSeconds);
  auto points = getPoints(timeRef, geoRef, navs, sampling);
}

}
}
