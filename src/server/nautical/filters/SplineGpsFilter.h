/*
 * SplineGpsFilter.h
 *
 *  Created on: 31 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_SPLINEGPSFILTER_H_
#define SERVER_NAUTICAL_FILTERS_SPLINEGPSFILTER_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/nautical/Ecef.h>
//#include <server/math/Spline.h>

namespace sail {
namespace SplineGpsFilter {

struct Settings {
  Duration<double> period = 2.0_s;
  Length<double> inlierThreshold = 1.0_s;
  double regWeight = 1.0;
};

class Curve {
public:
private:
  SmoothBoundarySplineBasis<double, 3> _basis;
  Array<double> coefs[3];
};

Array<GpsCurve>

}
}

#endif /* SERVER_NAUTICAL_FILTERS_SPLINEGPSFILTER_H_ */
