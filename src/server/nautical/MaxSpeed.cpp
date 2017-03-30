

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

Optional<TimedValue<Velocity<double>>> computeMaxSpeed(
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
