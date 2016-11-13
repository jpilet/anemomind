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
  Optional<Angle<double>> overrideCorrection;
};

struct Results {
  double objfValue = NAN;
  Angle<double> correction = 0.0_rad;

  Results() {}
  Results(double r, Angle<double> c) : objfValue(r),
      correction(c) {}
};

Results
  calibrateSingleChannel(
      SplineGpsFilter::EcefCurve curve,
      const Array<TimedValue<Angle<double>>> &headings,
      const Settings &settings = Settings(),
      DOM::Node *output = nullptr);

void makeCostPlot(
    int sampleCount,
    SplineGpsFilter::EcefCurve curve,
    const Array<TimedValue<Angle<double>>> &headings,
    DOM::Node *output,
    const Settings &settings = Settings());

template <typename T>
T evaluateFitness(
    const SplineGpsFilter::EcefCurve &curve,
    const Array<TimedValue<Angle<double>>> &headings,
    Angle<T> correction);


}
}

#endif /* SERVER_NAUTICAL_CALIB_MAGHDGCALIB_H_ */
