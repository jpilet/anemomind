#include "math.h"

namespace sail {


double rad2deg(double rad) {
  return 180.0*rad/M_PI;
}

double deg2rad(double deg) {
  return M_PI*deg/180.0;
}

double knots2MPS(double knots) {
  return 1852.0*knots/3600.0;
}

}
