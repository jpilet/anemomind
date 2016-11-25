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
  int sampleCount = 12;
  int relMinSampleCount = 5;
};

void makeAngleFitnessPlot(
    const SplineGpsFilter::EcefCurve &gpsCurve,
        const Array<TimedValue<Angle<double>>> &headings,
        const Array<Settings> &settings,
        DOM::Node *dst);

void makeFittedSinePlot(
    const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings,
    DOM::Node *dst);

Optional<Angle<double>> optimizeSineFit(
    const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings);

Optional<Angle<double>> optimizeSineFit(
    const Array<SplineGpsFilter::EcefCurve> &gpsCurve,
    const Array<Array<TimedValue<Angle<double>>>> &headings,
    const Settings &settings);

Array<TimedValue<Angle<double>>> applyCorrection(
    const Array<TimedValue<Angle<double>>> &src,
    Angle<double> corr);
Array<Array<TimedValue<Angle<double>>>> applyCorrection(
    const Array<Array<TimedValue<Angle<double>>>> &src,
    Angle<double> corr);

void makeSpreadPlot(
    const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings,
    DOM::Node *dst);

}
}

#endif /* SERVER_NAUTICAL_CALIB_MAGHDGCALIB2_H_ */
