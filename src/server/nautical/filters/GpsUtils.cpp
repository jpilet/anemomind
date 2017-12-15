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
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/Span.h>

namespace sail {
namespace GpsUtils {



namespace {
  auto pairThreshold = Duration<double>::seconds(12.0);
}

bool includeTime(TimeStamp x) {
  static auto lower = TimeStamp::UTC(2017, 9, 25, 11, 10, 0);
  static auto upper = TimeStamp::UTC(2017, 9, 25, 11, 14, 0);
  return lower <= x && x <= upper;
}


Array<TimedValue<HorizontalMotion<double> > > getGpsMotions(
    const NavDataset &ds,
    DOM::Node* log) {
  auto angle = ds.samples<GPS_BEARING>();
  auto speed = ds.samples<GPS_SPEED>();
  auto motions = makeMotionsFromVelocityAnglePairs(
      speed.begin(), speed.end(), angle.begin(), angle.end(), pairThreshold);
  Span<TimeStamp> sp;
  ArrayBuilder<TimedValue<HorizontalMotion<double>>> dst;
  for (auto x: motions) {
    if (includeTime(x.time)) {
      dst.add(x);
      sp.extend(x.time);
    }
  }

  LOG(INFO) << "Times from " << sp.minv().toString() << " to "
      << sp.maxv().toString();

  auto samples = dst.get();

  for (int i = 0; i < samples.size()-1; i++) {
    if (samples[i].time > samples[i+1].time) {
      DOM::addSubTextNode(log, "p",
          stringFormat(
              "SAMPLES are not chronologically ordered!!! at %d", i)).warning();
    }
  }

  DOM::addSubTextNode(log, "p", stringFormat("Span from %s to %s",
        sp.minv().toString().c_str(),
        sp.maxv().toString().c_str()));
  DOM::addSubTextNode(log, "p", stringFormat("Motion samples from %s to %s",
        samples.first().time.toString().c_str(),
        samples.last().time.toString().c_str()));

  return samples;
}

template <typename Container>
GeographicPosition<double> getReferencePositionSub(
    const Container &positions) {
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

GeographicPosition<double>
  getReferencePosition(
      const Array<TimedValue<GeographicPosition<double>>> &src) {
  return getReferencePositionSub<Array<TimedValue<GeographicPosition<double>>>>(src);
}

GeographicPosition<double>
  getReferencePosition(
      const TimedSampleRange<GeographicPosition<double>> &src) {
  return getReferencePositionSub<TimedSampleRange<GeographicPosition<double>>>(src);
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

std::shared_ptr<DispatchData> deduplicateGpsPositionsForChannel(
    DataCode code, std::string source,
    std::shared_ptr<DispatchData> src0) {
  if (code != GPS_POS) {
    return src0;
  }
  auto src = toTypedDispatchData<GPS_POS>(src0.get());
  const auto& srcValues = src->dispatcher()->values().samples();
  auto dst = std::make_shared<TypedDispatchDataReal<
      GeographicPosition<double>>>(
          code, source, nullptr, srcValues.size());

  auto dstValues = dst->dispatcher()->mutableValues();

  GeographicPosition<double> lastPos;
  for (auto x: srcValues) {
    if (!(x.value == lastPos)) {
      dstValues->append(x);
    }
    lastPos = x.value;
  }

  LOG(INFO) << "Deplicated GPS positions at " << source
      << " from " << srcValues.size() << " to "
      << dstValues->size();

  return dst;
}

NavDataset deduplicateGpsPositions(const NavDataset& ds) {
  return filterChannels(
      ds.dispatcher().get(), [](DataCode,std::string) {return true;},
      true, &deduplicateGpsPositionsForChannel);
}

}
}
