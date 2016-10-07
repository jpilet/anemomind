

#include <server/nautical/MaxSpeed.h>

#include <server/nautical/WGS84.h>

namespace sail {

Optional<MaxSpeed> computeMaxSpeed(const NavDataset& data, Duration<> delta) {
  TimedSampleRange<GeographicPosition<double>> pos = data.samples<GPS_POS>();

  Optional<Velocity<>> bestSpeed;
  TimeStamp bestTime, bestEnd;
  for (auto it = pos.begin(); it != pos.end(); ++it) {
    Optional<TimedValue<GeographicPosition<double>>> after =
      pos.nearest(it->time + delta);

    if (after.undefined()) {
      continue;
    }

    Velocity<> speed =
      distance(it->value, after.get().value) / (after.get().time - it->time);

    if (bestSpeed.undefined() || bestSpeed.get() < speed) {
      bestSpeed = speed;
      bestTime = it->time;
      bestEnd = after.get().time;
    }
  }
  if (bestSpeed.defined()) {
    return Optional<MaxSpeed>(MaxSpeed{
      bestSpeed.get(),
      bestTime,
      bestEnd
    });
  } else {
    return Optional<MaxSpeed>();
  }
}

}  // namespace sail
