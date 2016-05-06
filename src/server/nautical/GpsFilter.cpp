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

using namespace NavCompat;

Settings::Settings() :
      regWeight(12.0),
      motionWeight(1.0),
      samplingPeriod(Duration<double>::seconds(1.0)),
      inlierThreshold(Length<double>::meters(12)) {
  irlsSettings.initialWeight = 0.01;
  irlsSettings.iters = 30;
}

Duration<double> getLocalTime(TimeStamp timeRef, const Nav &nav) {
  return nav.time() - timeRef;
}

TimeStamp getGlobalTime(TimeStamp timeRef, Duration<double> localTime) {
  return timeRef + localTime;
}

Duration<double> getLocalTimeDif(const NavDataset &navs, int index) {
  auto li = getLastIndex(navs);
  if (index == 0) {
    return (getNav(navs, 1).time() - getNav(navs, 0).time());
  } else if (index == li) {
    return (getNav(navs, li).time() - getNav(navs, li-1).time());
  } else {
    return std::min(getNav(navs, index+1).time() - getNav(navs, index).time(),
                        getNav(navs, index).time() - getNav(navs, index-1).time());
  }
}


int navIndexToTimeIndex(int navIndex) {
  return 1 + 2*navIndex;
}

Nav middle(NavDataset navs) {
  return getNav(navs, getMiddleIndex(navs));
}

TimeStamp getTimeReference(NavDataset navs) {
  return middle(navs).time();
}

GeographicReference getGeographicReference(NavDataset navs) {
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
    GeographicReference geoRef, NavDataset navs, Sampling sampling) {
  int n = getNavSize(navs);
  std::vector<std::pair<Observation<2>, Observation<2> > > posAndVelPairs;
  posAndVelPairs.reserve(2*n);
  for (int i = 0; i < n; i++) {
    auto nav = getNav(navs, i);
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

    auto posObs = Observation<2>{weights,
      {localPos[0].meters(), localPos[1].meters()}};
    if (posObs.isFinite()) {
      auto velObs = Observation<2>{difWeights,
        {geoDif[0].meters(), geoDif[1].meters()}};
      if (velObs.isFinite()) {
        posAndVelPairs.push_back(std::pair<Observation<2>, Observation<2> >(
            posObs, velObs));
      }
    }

  }
  ArrayBuilder<Observation<2> > dst(2*posAndVelPairs.size());
  for (auto pair: posAndVelPairs) {
    dst.add(pair.first);
  }
  for (auto pair: posAndVelPairs) {
    dst.add(pair.second);
  }
  return dst.get();
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

Results filter(NavDataset navs, Settings settings) {
  if (getNavSize(navs) == 0) {
    LOG(WARNING) << "No Navs to filter";
    return Results();
  }
  assert(std::is_sorted(getBegin(navs), getEnd(navs)));

  auto timeRef = getTimeReference(navs);
  auto geoRef = getGeographicReference(navs);

  auto marg = Duration<double>::seconds(0.5);
  auto fromTime = getLocalTime(timeRef, getFirst(navs)) - marg;
  auto toTime = getLocalTime(timeRef, getLast(navs)) + marg;
  int sampleCount = 2 + int(floor((toTime - fromTime)/settings.samplingPeriod));
  Sampling sampling(sampleCount, fromTime.seconds(), toTime.seconds());
  Array<Observation<2> > observations = getObservations(settings,
      timeRef, geoRef, navs, sampling);
  CHECK(observations.size() % 2 == 0);

  if (observations.empty()) {
    LOG(WARNING) << "No valid observations";
    return Results();
  }

  auto results = DataFit::quadraticFitWithInliers(sampleCount, observations,
      settings.inlierThreshold.meters(), 2, settings.regWeight, settings.irlsSettings);

  auto reliableRange = getReliableSampleRange(observations, results.inliers);
  auto posObs = observations.sliceTo(getNavSize(navs));
  return Results{navs, posObs, sampling, results.samples, timeRef, geoRef, reliableRange};
}

bool isReliableW(Spani reliableSpan, const Sampling::Weights &w) {
  return reliableSpan.contains(w.lowerIndex) && reliableSpan.contains(w.upperIndex());
}

Arrayb Results::inlierMask() {
  assert(getNavSize(rawNavs) == positionObservations.size());
  return toArray(map(positionObservations, [&](const Observation<2> &obs) {
    return isReliableW(reliableSampleRange, obs.weights);
  }));
}


NavDataset Results::filteredNavs() const {
  int n = getNavSize(rawNavs);
  Array<Nav> dst = makeArray(rawNavs).dup();
  for (int i = 0; i < n; i++) {
    auto w = positionObservations[i].weights;
    auto &nav = dst[i];
    nav.setGeographicPosition(calcPosition(w));
    auto m = calcMotion(w);
    nav.setGpsBearing(m.angle());
    nav.setGpsSpeed(m.norm());
  }
  return fromNavs(dst);
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

bool Results::defined() const {
  return !Xmeters.empty();
}



}
}
