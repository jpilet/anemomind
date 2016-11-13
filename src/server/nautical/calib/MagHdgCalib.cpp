/*
 * MagHdgCalib.cpp
 *
 *  Created on: 13 Nov 2016
 *      Author: jonas
 */

#include "MagHdgCalib.h"

namespace sail {
namespace MagHdgCalib {

Array<TimedValue<Angle<double>>>
  calibrateSingleChannel(
    SplineGpsFilter::EcefCurve curve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings,
    DOM::Node *output) {

}

}
}
