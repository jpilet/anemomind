/*
 * MagHdgCalib.h
 *
 *  Created on: 13 Nov 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_MAGHDGCALIB_H_
#define SERVER_NAUTICAL_CALIB_MAGHDGCALIB_H_

#include <server/common/DOMUtils.h>
#include <server/common/TimedValue.h>
#include <server/nautical/filters/SplineGpsFilter.h>

namespace sail {
namespace MagHdgCalib {

struct Settings {
  Duration<double> windowSize = 2.0_minutes;
};


Array<TimedValue<Angle<double>>>
  calibrateSingleChannel(
    SplineGpsFilter::EcefCurve curve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings,
    DOM::Node *output);


}
}

#endif /* SERVER_NAUTICAL_CALIB_MAGHDGCALIB_H_ */
