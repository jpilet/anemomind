/*
 * WatSpeedCalib.cpp
 *
 *  Created on: 8 Dec 2016
 *      Author: jonas
 */

#include "WatSpeedCalib.h"

namespace sail {
namespace WatSpeedCalib {

Array<double> optimizeParameters(
    const Array<sail::SplineGpsFilter::EcefCurve> &gpsCurves,
    const Array<Array<TimedValue<Velocity<double>>>> &samples,
    const Corruptor &corruptor,
    const Settings &settings) {
  return Array<double>();
}

}
} /* namespace sail */
