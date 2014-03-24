#include "math.h"

namespace sail {

double floatMod(double a, double b) {
  if (a < 0) {
    return floatMod(a - (floor(a/b) - 3)*b, b);
  } else {
    return a - floor(a/b)*b;
  }
}

double angleAtRadians(double a, double b) {
  double a2 = floatMod(a, 2.0*M_PI);
  if (a2 >= b + M_PI) {
    return a2 - 2.0*M_PI;
  } else {
    return a2;
  }
}


}
