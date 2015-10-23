/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include "PowerMethod.h"

namespace sail {
namespace PowerMethod {

double sinAngle(const Eigen::VectorXd &a, const Eigen::VectorXd &b) {
  double cosAngle = a.dot(b)/(a.norm()*b.norm());
  double cos2Angle = cosAngle*cosAngle;
  return sqrt(std::max(0.0, 1.0 - cos2Angle));
}

Results computeMax(MatMul A, const Eigen::VectorXd &X, const Settings &settings) {
  double sinAngleTol = sin(settings.tol);
  Eigen::VectorXd Y = X;
  int i = 0;
  for (i = 0; i < settings.maxIters; i++) {
    Eigen::VectorXd Yprev = Y;
    Y = (1.0/Y.norm())*A(Y);
    if (sinAngle(Y, Yprev) < sinAngleTol) {
      break;
    }
  }
  return Results{Y, i};
}

Results computeMin(MatMul A, const Eigen::VectorXd &X, const Settings &s) {
  auto maxData = computeMax(A, X, s);
  auto maxEig = maxData.X.norm();
  auto temp = computeMax([&](const Eigen::VectorXd &x) {
    Eigen::VectorXd tmp = A(x);
    return Eigen::VectorXd(tmp - maxEig*x);
  }, X, s);
  double xNorm = temp.X.norm();
  return Results{(maxEig - xNorm)*(1.0/xNorm)*temp.X, temp.iters};
}

}
}
