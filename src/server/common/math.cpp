#include "math.h"
#include <assert.h>

namespace sail {

bool near(double a, double b, double marg) {
  double dif = std::abs(a - b);
  return dif <= marg;
}

bool nearWithNan(double a, double b, double marg) {
  if (std::isnan(a)) {
    return std::isnan(b);
  }
  return near(a, b, marg);
}

}
