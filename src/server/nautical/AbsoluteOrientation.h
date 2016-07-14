#ifndef NAUTICAL_ABSOLUTE_ORIENTATION_H
#define NAUTICAL_ABSOLUTE_ORIENTATION_H

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

struct AbsoluteOrientation {
  Angle<double> heading;
  Angle<double> roll;
  Angle<double> pitch;
};

bool isFinite(const AbsoluteOrientation &x);

}  // namespace sail

#endif  // NAUTICAL_ABSOLUTE_ORIENTATION_H
