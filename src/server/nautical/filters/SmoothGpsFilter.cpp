/*
 * SmoothGPSFilter.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/filters/GpsUtils.h>
#include <server/common/logging.h>
#include <server/math/nonlinear/CeresTrajectoryFilter.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Functional.h>
#include <server/math/Resampler.h>

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


Array<TimedValue<GeographicPosition<double> > >
  Results::getGlobalPositions() const {
  int n = localPositions.size();
  Array<TimedValue<GeographicPosition<double> > > dst(n);
  for (int i = 0; i < n; i++) {
    const auto &x = localPositions[i];
    auto &y = dst[i];
    y.time = x.time;
    y.value = geoRef.unmap(x.value);
  }
  return dst;
}


Results filterGpsData(const NavDataset &ds) {
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

  typedef CeresTrajectoryFilter::Types<2> Types;


  auto times = wrapIndexable<TypeMode::ConstRef>(samplingTimes);
  auto localPositions = wrapIndexable<TypeMode::ConstRef>(getLocalPositions(geoRef, positions));
  auto localMotions = wrapIndexable<TypeMode::ConstRef>(motions);

  using namespace CeresTrajectoryFilter;
  Settings settings;
  settings.huberThreshold = Length<double>::meters(12.0); // Sort of inlier threshold on the distance in meters
  settings.regWeight = 1.0;

  auto filtered = CeresTrajectoryFilter::filter<2>(
      (const AbstractArray<TimeStamp> &)times,
      (const AbstractArray<Types::TimedPosition> &)localPositions,
      (const AbstractArray<Types::TimedMotion> &)localMotions,
      settings, EmptyArray<Types::Position>());
  if (filtered.empty()) {
    LOG(ERROR) << "Failed to filter GPS data";
    return Results();
  }
  return Results{geoRef, filtered};
}

}
