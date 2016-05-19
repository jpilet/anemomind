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

namespace sail {

namespace {
  // All calculations in SI units.
  TimedSampleCollection<GeographicPosition<double> >::TimedVector
    buildPositions(TimeStamp timeRef, const GeographicReference &geoRef,
        const Array<TimedObservation<2> > &observations,
                const Array<Eigen::Vector2d> &data) {

    int n = observations.size();
    TimedSampleCollection<GeographicPosition<double> >::TimedVector dst;
    for (int i = 0; i < n; i++) {
      const auto &obs = observations[i];
      auto x = data[i];
      dst.push_back(TimedValue<GeographicPosition<double> >(
          timeRef + Duration<double>::seconds(obs.time),
          geoRef.unmap(GeographicReference::ProjectedPosition{
        Length<double>::meters(x(0)), Length<double>::meters(x(1))
      })));
    }
    return dst;
  }

  Array<TimedValue<HorizontalMotion<double> > > buildHorizontalMotions(
      TimeStamp timeRef, const Array<TimedObservation<2> > &observations,
      const Array<Eigen::Vector2d> &data) {
    int n = observations.size();
    ArrayBuilder<TimedValue<HorizontalMotion<double> > > dst(n);
    auto mps = Velocity<double>::metersPerSecond(1.0);
    for (int i = 1; i < n-1; i++) {
      auto spaceDiff = data[i+1] - data[i-1];
      auto timeDiff = observations[i+1].time - observations[i-1].time;
      auto motion = (1.0/timeDiff)*spaceDiff;
      dst.add(TimedValue<HorizontalMotion<double> >(
          timeRef + Duration<double>::seconds(observations[i].time),
          HorizontalMotion<double>(motion[0]*mps, motion[1]*mps)));
    }
    return dst.get();
  }

  TimedSampleCollection<Angle<double> >::TimedVector getAngles(
      const Array<TimedValue<HorizontalMotion<double> > > &motions) {
    int n = motions.size();

    TimedSampleCollection<Angle<double> >::TimedVector angles;
    for (int i = 0; i < n; i++) {
      const auto &x = motions[i];
      angles.push_back(TimedValue<Angle<double> >(x.time,
          toFinite(x.value.angle(), Angle<double>::radians(0.0))));
    }
    return angles;
  }

  TimedSampleCollection<Velocity<double> >::TimedVector getVelocities(
      const Array<TimedValue<HorizontalMotion<double> > > &motions) {
    int n = motions.size();

    TimedSampleCollection<Velocity<double> >::TimedVector angles;
    for (int i = 0; i < n; i++) {
      const auto &x = motions[i];
      angles.push_back(TimedValue<Velocity<double> >(x.time,
          x.value.norm()));
    }
    return angles;
  }
}

std::shared_ptr<Dispatcher> filterGpsData(const NavDataset &ds) {
  if (ds.isDefaultConstructed()) {
    LOG(WARNING) << "Nothing to filter";
    return std::shared_ptr<Dispatcher>();
  }

  auto motions = GpsUtils::getGpsMotions(ds);
  auto positions = ds.samples<GPS_POS>();
  if (positions.empty()) {
    LOG(ERROR) << "No GPS positions in dataset, cannot filter";
    return std::shared_ptr<Dispatcher>();
  }
  auto referencePosition = GpsUtils::getReferencePosition(positions);
  auto referenceTime = GpsUtils::getReferenceTime(positions);
  GeographicReference geoRef(referencePosition);

  auto localMotions = GpsUtils::toLocalObservations(referenceTime, motions);
  auto localPositions = GpsUtils::toLocalObservations(geoRef, referenceTime, positions);

  int n = localMotions.size() + localPositions.size();
  Array<TimedObservation<2> > allObservations(n);

  auto endOfMerge = std::merge(localMotions.begin(), localMotions.end(),
      localPositions.begin(), localPositions.end(), allObservations.begin());
  CHECK(endOfMerge == allObservations.end());

  // TODO: use server/math/Resampler.h to generate samples.
  Arrayd samplingTimes = GpsUtils::getSamplingTimes(allObservations);

  using namespace CeresTrajectoryFilter;
  Settings settings;
  settings.huberThreshold = 12.0; // Sort of inlier threshold on the distance in meters
  settings.regWeight = 1.0;
  auto filtered = CeresTrajectoryFilter::filter(
      samplingTimes,
      allObservations, settings, Array<Eigen::Vector2d>());
  if (filtered.empty()) {
    LOG(ERROR) << "Failed to filter GPS data";
    return std::shared_ptr<Dispatcher>();
  }
  CHECK(filtered.size() == allObservations.size());
  auto filteredPositions = buildPositions(referenceTime, geoRef, allObservations, filtered);


  auto filteredMotions = buildHorizontalMotions(referenceTime, allObservations, filtered);
  // TODO: Splitting the motions like this feels bad. Would it be useful to add a
  // channel for GPS_MOTION, or something?
  auto velocities = getVelocities(filteredMotions);
  auto angles = getAngles(filteredMotions);

  const char *srcName = "SmoothGpsFilter";

  auto dst = std::make_shared<Dispatcher>();

  dst->set(GPS_POS, srcName,
      makeDispatchDataFromSamples<GPS_POS>(srcName, filteredPositions));

  dst->set(GPS_BEARING, srcName,
      makeDispatchDataFromSamples<GPS_BEARING>(srcName, angles));

  dst->set(GPS_SPEED, srcName,
      makeDispatchDataFromSamples<GPS_SPEED>(srcName, velocities));

  return dst;
}

}
