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
#include <server/common/TimeMapper.h>
#include <server/math/Spline.h>
#include <server/common/TimedValue.h>
#include <server/nautical/GeographicPosition.h>

namespace sail {
namespace SplineGpsFilter {

struct Settings {
  Duration<double> period = 2.0_s;
  Length<double> inlierThreshold = 12.0_m;
  double regWeight = 1.0;
};

class Curve {
public:
private:
  TimeMapper _timeMapper;
  SmoothBoundarySplineBasis<double, 3> _basis;
  Array<double> coefs[3];
};

Array<Curve> filter(
    const Array<TimedValue<GeographicPosition<double>>> &positionData,
    const Array<TimedValue<HorizontalMotion<double>>> &motionData,
    const TimeMapper &segments,
    const Settings &settings);

}
}

#endif /* SERVER_NAUTICAL_FILTERS_SPLINEGPSFILTER_H_ */
