#ifndef NAUTICAL_MAX_SPEED_H
#define NAUTICAL_MAX_SPEED_H

#include <server/nautical/NavDataset.h>

namespace sail {

struct MaxSpeed {
  Velocity<double> speed;
  TimeStamp begin;
  TimeStamp end;
};

Optional<MaxSpeed> computeMaxSpeed(const NavDataset& data, Duration<> delta);

}  // namespace sail

#endif  // NAUTICAL_MAX_SPEED_H
