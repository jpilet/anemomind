

#include <server/nautical/MaxSpeed.h>

#include <server/nautical/WGS84.h>

namespace sail {

namespace {
  Optional<TimedValue<Velocity<double>>> maxSpeed(
      const Optional<TimedValue<Velocity<double>>> &x,
      const TimedValue<Velocity<double>> &y) {
    if (x.defined() && x.get().value >= y.value) {
      return x;
    }
    return y;
  }
}

Optional<TimedValue<Velocity<double>>> computeMaxSpeed(
    const NavDataset& data) {
  auto speeds = data.samples<GPS_SPEED>();
  Optional<TimedValue<Velocity<double>>> best;
  for (auto x: speeds) {
    best = maxSpeed(best, x);
  }
  return best;
}

}  // namespace sail
