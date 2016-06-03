/*
 * SmoothGPSFilter.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <server/common/ArrayBuilder.h>
#include <server/common/Functional.h>
#include <server/common/logging.h>
#include <server/math/nonlinear/CeresTrajectoryFilter.h>
#include <server/math/Resampler.h>
#include <server/nautical/filters/GpsUtils.h>
#include <server/nautical/filters/SmoothGpsFilter.h>

namespace sail {

namespace {
  template <typename Range>
  Array<TimeStamp> getTimeStamps(const Range &r) {
    ArrayBuilder<TimeStamp> dst(r.end() - r.begin());
    for (auto x: r) {
      dst.add(x.time);
    }
    return dst.get();
  }

  Array<TimeStamp> buildSampleTimes(
      const Array<TimeStamp> &positionTimes,
      const Array<TimeStamp> &motionTimes, Duration<double> period) {
    Array<TimeStamp> all(positionTimes.size() + motionTimes.size());
    CHECK(std::merge(positionTimes.begin(), positionTimes.end(),
        motionTimes.begin(), motionTimes.end(), all.begin()) == all.end());
    return Resampler::resample(all, period);
  }

  Array<CeresTrajectoryFilter::Types<2>::TimedPosition> getLocalPositions(
      const GeographicReference &geoRef,
      const TimedSampleRange<GeographicPosition<double> > &positions) {
    int n = positions.size();
    Array<CeresTrajectoryFilter::Types<2>::TimedPosition> dst(n);
    for (int i = 0; i < n; i++) {
      auto &y = dst[i];
      auto x = positions[i];
      y.time = x.time;
      y.value = geoRef.map(x.value);
    }
    return dst;
  }
}


TimedSampleCollection<GeographicPosition<double> >
  Results::getGlobalPositions() const {
  int n = localPositions.size();
  TimedSampleCollection<GeographicPosition<double> >::TimedVector dst;
  dst.resize(n);
  for (int i = 0; i < n; i++) {
    const auto &x = localPositions[i];
    auto &y = dst[i];
    y.time = x.time;
    y.value = geoRef.unmap(x.value);
  }
  return TimedSampleCollection<GeographicPosition<double> >(dst);
}

typedef CeresTrajectoryFilter::Types<2> FTypes;

Array<FTypes::TimedMotion> toLocalMotions(
    const Array<TimedValue<HorizontalMotion<double>>> &motions) {
  int n = motions.size();
  Array<FTypes::TimedMotion> dst(n);
  for (int i = 0; i < n; i++) {
    const auto &m = motions[i];
    dst[i] = FTypes::TimedMotion(
        m.time, FTypes::Motion{m.value[0], m.value[1]});
  }
  return dst;
}

CeresTrajectoryFilter::Settings makeDefaultSettings() {
  CeresTrajectoryFilter::Settings settings;
  settings.huberThreshold = Length<double>::meters(12.0); // Sort of inlier threshold on the distance in meters
  settings.regWeight = 10.0;
  return settings;
}

Results filterGpsData(const NavDataset &ds,
    const CeresTrajectoryFilter::Settings &settings) {
  if (ds.isDefaultConstructed()) {
    LOG(WARNING) << "Nothing to filter";
    return Results();
  }

  Duration<double> samplingPeriod = Duration<double>::seconds(1.0);

  auto motions = GpsUtils::getGpsMotions(ds);
  auto positions = ds.samples<GPS_POS>();
  if (positions.empty()) {
    LOG(ERROR) << "No GPS positions in dataset, cannot filter";
    return Results();
  }
  auto referencePosition = GpsUtils::getReferencePosition(positions);
  GeographicReference geoRef(referencePosition);



  auto positionTimes = getTimeStamps(positions);
  auto motionTimes = getTimeStamps(motions);

  auto samplingTimes = buildSampleTimes(positionTimes, motionTimes,
      samplingPeriod);


  IndexableWrap<Array<TimeStamp>, TypeMode::ConstRef> times =
      wrapIndexable<TypeMode::ConstRef>(samplingTimes);
  IndexableWrap<Array<FTypes::TimedPosition>, TypeMode::Value> localPositions
    = wrapIndexable<TypeMode::Value>(getLocalPositions(geoRef, positions));
  IndexableWrap<Array<FTypes::TimedMotion>, TypeMode::Value> localMotions
    = wrapIndexable<TypeMode::Value>(toLocalMotions(motions));

  using namespace CeresTrajectoryFilter;

  typedef FTypes::TimedPosition TimedPosition;
  typedef FTypes::TimedMotion TimedMotion;

  const AbstractArray<TimeStamp> &t = times;
  const AbstractArray<TimedPosition> &p = localPositions;
  const AbstractArray<TimedMotion> &m = localMotions;

  auto e = EmptyArray<FTypes::Position>();

  Types<2>::TimedPositionArray filtered = CeresTrajectoryFilter::filter<2>(
      t, p, m,
      settings, e);
  if (filtered.empty()) {
    LOG(ERROR) << "Failed to filter GPS data";
    return Results();
  }

  return Results{geoRef, filtered};
}

}
