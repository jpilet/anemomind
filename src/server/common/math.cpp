#include "math.h"
#include <assert.h>

namespace sail {


double localizeAngleRadians(double a, double b) {
  // Currently, the code only supports this case:
  assert(0 <= b);
  assert(b < 2.0*M_PI);

  double a2 = positiveMod(a, 2.0*M_PI);
  if (a2 >= b + M_PI) {
    return a2 - 2.0*M_PI;
  } else {
    return a2;
  }
}


}
