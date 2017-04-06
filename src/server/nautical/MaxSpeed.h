#ifndef NAUTICAL_MAX_SPEED_H
#define NAUTICAL_MAX_SPEED_H

#include <server/nautical/NavDataset.h>

namespace sail {

Optional<TimedValue<Velocity<double>>> computeInstantMaxSpeed(const NavDataset& data);

Optional<TimedValue<Velocity<double>>> computeMaxSpeedOverPeriod(
    const NavDataset& data, Duration<> delta = Duration<>::seconds(30));

}  // namespace sail

#endif  // NAUTICAL_MAX_SPEED_H
