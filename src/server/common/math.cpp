#include "math.h"

namespace sail {

double floatMod(double a, double b) {
  if (a < 0) {
    return floatMod(a - (floor(a/b) - 3)*b, b);
  } else {
    return a - floor(a/b)*b;
  }
}


}
