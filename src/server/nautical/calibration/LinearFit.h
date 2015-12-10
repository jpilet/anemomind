/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_LINEARFIT_H_
#define SERVER_NAUTICAL_CALIBRATION_LINEARFIT_H_

#include <server/math/EigenUtils.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/nautical/CalibrationBenchmark.h>
#include <server/nautical/calibration/LinearCalibration.h>

namespace sail {
namespace LinearFit {

EigenUtils::MatrixPair buildNormalEqs(
    Angle<double> heading,
    const Eigen::MatrixXd &A1, const Eigen::MatrixXd &B1);

EigenUtils::MatrixPair buildNormalEqs(
    Angle<double> heading,
    const Eigen::MatrixXd &AB1);

EigenUtils::MatrixPair makeXYCoefMatrices(EigenUtils::MatrixPair Xflow,
                                          EigenUtils::MatrixPair Yflow);

Array<EigenUtils::MatrixPair> makeNormalEqs(Array<Angle<double> > headings,
                                            EigenUtils::MatrixPair flowEqs,
                                            int dim);

Array<EigenUtils::MatrixPair> makeCoefMatrices(Array<EigenUtils::MatrixPair> X,
                                               Array<EigenUtils::MatrixPair> Y,
                                               Array<Spani> spans);

Eigen::VectorXd minimizeLeastSquares(Array<EigenUtils::MatrixPair> coefMatrices);
Eigen::VectorXd minimizeLeastSumOfNorms(Array<EigenUtils::MatrixPair> coefMatrices);

Eigen::VectorXd minimizeCoefs(Array<EigenUtils::MatrixPair> coefMatrices, int exponent);

struct Settings {
 int exponent = 2;
 double relativeStep = 0.5;
 int spanSize = 100;
};

class LinearFitCorrectorFunction : public CorrectorFunction {
 public:
  LinearFitCorrectorFunction(const Settings &s,
                             LinearCalibration::FlowSettings flowSettings) : _settings(s),
                               _flowSettings(flowSettings) {}

  Array<CalibratedNav<double> > operator()(const Array<Nav> &navs) const;
  std::string toString() const {
    return "[calibration/LinearFit]";
  }
 private:
  Settings _settings;
  LinearCalibration::FlowSettings _flowSettings;
};

}
}

#endif
