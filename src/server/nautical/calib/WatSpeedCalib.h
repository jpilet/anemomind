/*
 * WatSpeedCalib.h
 *
 *  Created on: 8 Dec 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_WATSPEEDCALIB_H_
#define SERVER_NAUTICAL_CALIB_WATSPEEDCALIB_H_

#include <ceres/jet.h>
#include <server/common/Array.h>
#include <server/nautical/filters/SplineGpsFilter.h>

namespace sail {
namespace WatSpeedCalib {

typedef ceres::Jet<double, 1> AD;

class Corruptor {
public:
  virtual Array<double> initialParameters() = 0;
  virtual AD eval(const Array<AD> &parameters, AD value) = 0;
  virtual ~Corruptor() {}
};

struct Settings {
  int iterations = 10;
  int windowSize = 100;
};

Array<double> optimizeParameters(
    const Array<sail::SplineGpsFilter::EcefCurve> &gpsCurves,
    const Array<Array<TimedValue<Velocity<double>>>> &samples,
    const Corruptor &corruptor,
    const Settings &settings);

}
}

#endif /* SERVER_NAUTICAL_CALIB_WATSPEEDCALIB_H_ */
