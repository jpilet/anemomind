/*
 * SplineGpsFilter.cpp
 *
 *  Created on: 31 Oct 2016
 *      Author: jonas
 */

#include "SplineGpsFilter.h"

namespace sail {
namespace SplineGpsFilter {

Curve::Curve(const TimeMapper &mapper) : _timeMapper(mapper),
    _coefs(Array<double>::fill(mapper.sampleCount*3, 0.0)),
    _basis(mapper.sampleCount) {}

double *Curve::ptr() {
  return _coefs.ptr();
}

Array<Curve> filter(
    const Array<TimedValue<GeographicPosition<double>>> &positionData,
    const Array<TimedValue<HorizontalMotion<double>>> &motionData,
    const Array<TimeMapper> &segments,
    const Settings &settings) {

}



}
}
