/*
 * GpsUtils.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <server/nautical/filters/GpsUtils.h>
#include <server/nautical/filters/MotionsFromPairs.h>
#include <server/nautical/WGS84.h>
#include <server/math/nonlinear/SpatialMedian.h>
#include <server/nautical/InvWGS84.h>

namespace sail {
namespace GpsUtils {



namespace {
  auto pairThreshold = Duration<double>::seconds(12.0);
}

Array<TimedValue<HorizontalMotion<double> > > getGpsMotions(const NavDataset &ds) {
  auto angle = ds.samples<GPS_BEARING>();
  auto speed = ds.samples<GPS_SPEED>();
  return makeMotionsFromVelocityAnglePairs(
      speed.begin(), speed.end(), angle.begin(), angle.end(), pairThreshold);
}

GeographicPosition<double> getReferencePosition(
    const TimedSampleRange<GeographicPosition<double> > &positions) {
  int n = positions.size();
  Array<Eigen::Vector3d> pos3d(n);
  auto unit = Length<double>::meters(1.0);
  for (int i = 0; i < n; i++) {
    Length<double> xyz[3];
    WGS84<double>::toXYZ(positions[i].value, xyz);
    pos3d[i] = Eigen::Vector3d(xyz[0]/unit, xyz[1]/unit, xyz[2]/unit);
  }
  SpatialMedian::Settings settings;
  settings.iters = 4;
  auto median = SpatialMedian::compute(pos3d, settings);
  Length<double> xyz[3] = {median(0)*unit, median(1)*unit, median(2)*unit};
  return computeGeographicPositionFromXYZ(xyz);
}

TimeStamp getReferenceTime(
    const TimedSampleRange<GeographicPosition<double> > &positions) {
  if (positions.empty()) {
    LOG(WARNING) << "Empty positions means undefined time";
    return TimeStamp();
  }

  int middle = positions.size()/2;
  return positions[middle].time;
}

namespace {
  TimedObservation<2> toLocalObservation(
      const GeographicReference &geoRef, TimeStamp timeReference,
      const TimedValue<GeographicPosition<double> > &geoPos) {
    auto projected = geoRef.map(geoPos.value);
    auto localTime = (geoPos.time - timeReference).seconds();
    return TimedObservation<2>{
      0, localTime,
      (Eigen::Vector2d(projected[0].meters(), projected[1].meters()))
    };
  }

  TimedObservation<2> toLocalObservation(
      TimeStamp timeReference,
      const TimedValue<HorizontalMotion<double> > &motion) {
    return TimedObservation<2>{
      1, (motion.time - timeReference).seconds(),
      (Eigen::Vector2d(motion.value[0].metersPerSecond(),
          motion.value[1].metersPerSecond()))
    };
  }
}


Array<TimedObservation<2> > toLocalObservations(
      const GeographicReference &geoRef, TimeStamp timeReference,
      const TimedSampleRange<GeographicPosition<double> > &positions) {
  int n = positions.size();
  Array<TimedObservation<2> > dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = toLocalObservation(geoRef, timeReference, positions[i]);
  }
  return dst;
}

Array<TimedObservation<2> > toLocalObservations(TimeStamp timeReference,
    const Array<TimedValue<HorizontalMotion<double> > > &motions) {
  int n = motions.size();
  Array<TimedObservation<2> > dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = toLocalObservation(timeReference, motions[i]);
  }
  return dst;
}

Arrayd getSamplingTimes(const Array<TimedObservation<2> > &observations) {

  // TODO: call 'resample' in server/math/Resampler.h here.

  int n = observations.size();
  Arrayd dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = observations[i].time;
  }
  return dst;
}


}
}
