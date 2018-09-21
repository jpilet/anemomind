#ifndef NAUTICAL_ABSOLUTE_ORIENTATION_H
#define NAUTICAL_ABSOLUTE_ORIENTATION_H

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

struct AbsoluteOrientation {
  Angle<double> heading;
  Angle<double> roll;
  Angle<double> pitch;

  bool operator==(const AbsoluteOrientation& other) const {
    return heading == other.heading &&
        roll == other.roll && pitch == other.pitch;
  }

#ifdef ON_SERVER
  std::string str() const {
    return "{ h:" +  heading.str() + ", r:", roll.str()
      + ", p:" + pitch.str() + " }";
  }
#endif
};

bool isFinite(const AbsoluteOrientation &x);

}  // namespace sail

#endif  // NAUTICAL_ABSOLUTE_ORIENTATION_H
