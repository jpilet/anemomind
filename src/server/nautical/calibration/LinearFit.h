/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_LINEARFIT_H_
#define SERVER_NAUTICAL_CALIBRATION_LINEARFIT_H_

#include <server/math/EigenUtils.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

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

}
}

#endif
