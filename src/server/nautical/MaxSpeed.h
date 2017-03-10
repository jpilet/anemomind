#ifndef NAUTICAL_MAX_SPEED_H
#define NAUTICAL_MAX_SPEED_H

#include <server/nautical/NavDataset.h>

namespace sail {


Optional<TimedValue<Velocity<double>>> computeMaxSpeed(const NavDataset& data);

}  // namespace sail

#endif  // NAUTICAL_MAX_SPEED_H
