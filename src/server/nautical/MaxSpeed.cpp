

#include <server/nautical/MaxSpeed.h>

#include <server/nautical/WGS84.h>

namespace sail {

namespace {
  TimedValue<Velocity<double>> maxSpeed(
      const TimedValue<Velocity<double>> &x,
      const TimedValue<Velocity<double>> &y) {
    return x.value < y.value? y : x;
  }
}

Optional<TimedValue<Velocity<double>>> computeMaxSpeedOverPeriod(
    const NavDataset& data, Duration<> delta) {
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
    return makeOptional(TimedValue<Velocity<double>>(
            (bestTime + (bestEnd - bestTime) * .5),
            bestSpeed.get()));
  } else {
    return Optional<TimedValue<Velocity<double>>>();
  }
}

Optional<TimedValue<Velocity<double>>> computeInstantMaxSpeed(
    const NavDataset& data) {
  auto speeds = data.samples<GPS_SPEED>();
  if (speeds.empty()) {
    return Optional<TimedValue<Velocity<double>>>();
  }
  TimedValue<Velocity<double>> best = speeds[0];
  for (auto x: speeds) {
    CHECK(!isNaN(x.value));
    best = maxSpeed(best, x);
  }
  return best;
}

}  // namespace sail
