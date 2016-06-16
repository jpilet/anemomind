/*
 * GpsUtils.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <server/math/nonlinear/SpatialMedian.h>
#include <server/nautical/filters/GpsUtils.h>
#include <server/nautical/filters/MotionsFromPairs.h>
#include <server/nautical/InvWGS84.h>
#include <server/nautical/WGS84.h>

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
  settings.iters = 8;
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

}
}
