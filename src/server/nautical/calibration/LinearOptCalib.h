/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_LINEAROPTCALIB_H_
#define SERVER_NAUTICAL_CALIBRATION_LINEAROPTCALIB_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/math/irls.h>
#include <server/nautical/calibration/LinearCalibration.h>
#include <Eigen/Core>

namespace sail {
namespace LinearOptCalib {

Eigen::MatrixXd makeLhs(const Eigen::MatrixXd &A, Array<Spani> spans);
Eigen::SparseMatrix<double> makeRhs(const Eigen::VectorXd &B, Array<Spani> spans);


struct Settings {
  Angle<double> inlierThresh;
  irls::Settings irlsSettings;
};

struct Results {
  Arrayd parameters;
};

Results optimize(
    const Eigen::MatrixXd &A, const Eigen::VectorXd &B,
    Array<Spani> spans,
    const Settings &settings);




}
}

#endif /* SERVER_NAUTICAL_CALIBRATION_LINEAROPTCALIB_H_ */
