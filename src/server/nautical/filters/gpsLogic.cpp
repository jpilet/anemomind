#include <server/nautical/filters/gpsLogic.h>

#include <algorithm>
#include <server/nautical/DownSampleGps.h>
#include <server/nautical/ValidPeriods.h>
#include <server/nautical/WGS84.h>

namespace sail {

namespace {

void linkFromBack(
    const TimedSampleCollection<GeographicPosition<double>>& p,
    StatusTimedVector *segments,
    TimedSampleCollection<GeographicPosition<double>>::TimedVector* keptPos,
    const GpsLogicParams& params) {
  using TimedVector = TimedSampleCollection<GeographicPosition<double>>::TimedVector;

  const TimedVector& samples(p.samples());

  segments->clear();

  bool status = false;
  TimeStamp earliestLinedTime;

  for (auto after = samples.rbegin(); after != samples.rend(); ) {
    // search for a matching measurement, ignoring high freq outliers
    bool foundLink = false;
    auto lastChecked = after;
    for (auto before = after + 1;
         before != samples.rend()
         && (after->time - before->time) <= params.maxLinkTime; ++before) {
      lastChecked = before;
      const Length<> d(distance(before->value, after->value));
      const Velocity<> speed(d / (after->time - before->time));
      if (d < params.maxLinkDist
          && speed < params.maxLinkSpeed) {
        foundLink = true;
        earliestLinedTime = before->time;
        if (!status) {
          status = true;
          segments->push_front(TimedValue<StatusChange>(
                  after->time, StatusChange::toInvalid));
          keptPos->push_front(*after);
        }
        keptPos->push_front(*before);
        break;
      } else {
        if (params.logRejectedPoints) {
          LOG(INFO) << "Rejecting point at time: " << before->time
            << " dist: " << d.meters() << "m (limit: " << params.maxLinkDist.meters()
            << "m), speed: " << speed.knots() << "kn (limit: " << params.maxLinkSpeed.knots();
        }
      }
    }

    if (!foundLink) {
      // 'after' can't be linked to anything.
      if (status) {
        segments->push_front(TimedValue<StatusChange>(
                after->time, StatusChange::toValid));
        status = false;
      }
    }

    after = lastChecked + 1;
  }

  if (status) {
    segments->push_front(TimedValue<StatusChange>(
                earliestLinedTime, StatusChange::toValid));
  }
}

}  // namespace

NavDataset findGpsSegments(const NavDataset& in,
                           const GpsLogicParams& params,
                           StatusTimedVector* segments) {
  // For each GPS channel, connect measurements to determine when we have data.
  // Connections goes backward in time. "Teleportations" are removed by killing
  // anterior data. This is because we assume that such jumps only occur when
  // GPS initialize and acquire satellites.
  std::vector<std::string> sources = in.sourcesForChannel(GPS_POS);

  NavDataset result = in.stripChannel(GPS_POS);

  for (const std::string& src: sources) {
    TimedSampleCollection<GeographicPosition<double>> track(
        getCleanGpsPositions(in, src));
    StatusTimedVector trackSegments;

    TimedSampleCollection<GeographicPosition<double>>::TimedVector keptPos;
    linkFromBack(track, &trackSegments, &keptPos, params);

    if (trackSegments.size() > 0) {
      *segments = statusVectorUnion(*segments, trackSegments);

      result = result.addChannel<GeographicPosition<double>>(
          GPS_POS, src + " gpsLogic", keptPos);
    }
  }

  return result;
}

}  // namespace sail
