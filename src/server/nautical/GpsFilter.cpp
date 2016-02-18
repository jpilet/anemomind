/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/GpsFilter.h>
#include <server/nautical/GeographicReference.h>
#include <server/common/Span.h>
#include <server/math/nonlinear/DataFit.h>
#include <algorithm>
#include <server/common/ArrayIO.h>


namespace sail {
namespace GpsFilter {

Settings::Settings() :
      regWeight(12.0),
      motionWeight(1.0),
      samplingPeriod(Duration<double>::seconds(1.0)),
      inlierThreshold(Length<double>::meters(12)) {
  irlsSettings.logWeighting = true;
  irlsSettings.initialWeight = 0.01;
  irlsSettings.iters = 30;
}

Duration<double> getLocalTime(TimeStamp timeRef, const Nav &nav) {
  return nav.time() - timeRef;
}

TimeStamp getGlobalTime(TimeStamp timeRef, Duration<double> localTime) {
  return timeRef + localTime;
}

Duration<double> getLocalTimeDif(const NavCollection &navs, int index) {
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

Nav middle(NavCollection navs) {
  return navs[navs.middle()];
}

TimeStamp getTimeReference(NavCollection navs) {
  return middle(navs).time();
}

GeographicReference getGeographicReference(NavCollection navs) {
  return GeographicReference(middle(navs).geographicPosition());
}

GeographicReference::ProjectedPosition integrate(
    const HorizontalMotion<double> motion, Duration<double> dur) {
  auto x = Length<double>::meters(motion[0].metersPerSecond()*dur.seconds());
  auto y = Length<double>::meters(motion[1].metersPerSecond()*dur.seconds());
  return GeographicReference::ProjectedPosition{x, y};
}



Array<Observation<2> > getObservations(
    Settings settings,
    TimeStamp timeRef,
    GeographicReference geoRef, NavCollection navs, Sampling sampling) {
  int n = navs.size();
  Array<Observation<2> > dst(2*n);
  for (int i = 0; i < n; i++) {
    auto nav = navs[i];
    auto localTime = getLocalTime(timeRef, nav);
    auto localTimeDif = getLocalTimeDif(navs, i);
    auto difScale = settings.motionWeight*localTimeDif.seconds()/sampling.period();
    auto weights = sampling.represent(localTime.seconds());
    auto geoDif = integrate(nav.gpsMotion(), localTimeDif);
    int offset = 2*i;
    auto localPos = geoRef.map(nav.geographicPosition());
    auto difWeights = weights;
    difWeights.lowerWeight = -difScale;
    difWeights.upperWeight = difScale;

    for (int i = 0; i < 2; i++) {
      CHECK(saneCalculation(localPos[i].meters(), Arrayd{
              nav.geographicPosition().lon().degrees(),
              nav.geographicPosition().lat().degrees(),
            }));
      CHECK(saneCalculation(geoDif[i].meters(), Arrayd{
        nav.gpsMotion()[0].metersPerSecond(),
        nav.gpsMotion()[1].metersPerSecond()
      }));
    }

    // Based on the position
    dst[i] = Observation<2>{weights,
      {localPos[0].meters(), localPos[1].meters()}};

    // Based on the speed
    dst[n + i] = Observation<2>{difWeights,
      {geoDif[0].meters(), geoDif[1].meters()}};
  }
  return dst;
}

Spani getReliableSampleRange(Array<Observation<2> > observations_, Arrayb inliers_) {
  assert(observations_.size() == inliers_.size());
  int n = inliers_.size()/2;
  assert(2*n == inliers_.size());

  auto inliers = inliers_.sliceTo(n);
  auto observations = observations_.sliceTo(n);

  Spani range;
  for (int i = 0; i < n; i++) {
    if (inliers[i]) {
      auto obs = observations[i];
      range.extend(obs.weights.lowerIndex);
      range.extend(obs.weights.upperIndex());
    }
  }
  return range;
}

Results filter(NavCollection navs, Settings settings) {
  assert(std::is_sorted(navs.begin(), navs.end()));
  auto timeRef = getTimeReference(navs);
  auto geoRef = getGeographicReference(navs);

  auto marg = Duration<double>::seconds(0.5);
  auto fromTime = getLocalTime(timeRef, navs.first()) - marg;
  auto toTime = getLocalTime(timeRef, navs.last()) + marg;
  int sampleCount = 2 + int(floor((toTime - fromTime)/settings.samplingPeriod));
  Sampling sampling(sampleCount, fromTime.seconds(), toTime.seconds());
  auto observations = getObservations(settings,
      timeRef, geoRef, navs, sampling);



  auto results = DataFit::quadraticFitWithInliers(sampleCount, observations,
      settings.inlierThreshold.meters(), 2, settings.regWeight, settings.irlsSettings);

  auto reliableRange = getReliableSampleRange(observations, results.inliers);
  auto posObs = observations.sliceTo(navs.size());
  return Results{navs, posObs, sampling, results.samples, timeRef, geoRef, reliableRange};
}

bool isReliableW(Spani reliableSpan, const Sampling::Weights &w) {
  return reliableSpan.contains(w.lowerIndex) && reliableSpan.contains(w.upperIndex());
}

Arrayb Results::inlierMask() {
  assert(rawNavs.size() == positionObservations.size());
  return toArray(map(positionObservations, [&](const Observation<2> &obs) {
    return isReliableW(reliableSampleRange, obs.weights);
  }));
}


NavCollection Results::filteredNavs() const {
  int n = rawNavs.size();
  Array<Nav> dst = rawNavs.makeArray().dup();
  for (int i = 0; i < n; i++) {
    auto w = positionObservations[i].weights;
    auto &nav = dst[i];
    nav.setGeographicPosition(calcPosition(w));
    auto m = calcMotion(w);
    nav.setGpsBearing(m.angle());
    nav.setGpsSpeed(m.norm());
  }
  return NavCollection::fromNavs(dst);
}

Sampling::Weights Results::calcWeights(TimeStamp t) const {
  return sampling.represent((t - timeRef).seconds());
}

HorizontalMotion<double> Results::calcMotion(const Sampling::Weights &w) const {
  double deriv[2];
  double f = 1.0/sampling.period();
  w.evalDerivative(Xmeters, deriv);
  return HorizontalMotion<double>{
    Velocity<double>::metersPerSecond(f*deriv[0]),
    Velocity<double>::metersPerSecond(f*deriv[1])
  };
}

GeographicPosition<double> Results::calcPosition(const Sampling::Weights &w) const {
  double pos[2];
  w.eval(Xmeters, pos);
  return geoRef.unmap(GeographicReference::ProjectedPosition{
    Length<double>::meters(pos[0]),
    Length<double>::meters(pos[1])});
}


}
}
