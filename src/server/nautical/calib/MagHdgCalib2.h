/*
 * MagHdgCalib2.h
 *
 *  Created on: 13 Nov 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_MAGHDGCALIB2_H_
#define SERVER_NAUTICAL_CALIB_MAGHDGCALIB2_H_

#include <server/nautical/filters/SplineGpsFilter.h>


namespace sail {
namespace DOM {class Node;}

namespace MagHdgCalib2 {

struct Settings {
  int windowSize = 100;
  int plotSampleCount = 30;
};

void makeAngleFitnessPlot(
    const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Array<Settings> &settings,
    DOM::Node *dst);

}
}

#endif /* SERVER_NAUTICAL_CALIB_MAGHDGCALIB2_H_ */
