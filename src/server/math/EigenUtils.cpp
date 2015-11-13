/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/math/EigenUtils.h>
#include <server/math/Random.h>

namespace sail {
namespace EigenUtils {

Eigen::MatrixXd makeRandomMatrix(int rows, int cols,
    RandomEngine *rng, double s) {
  std::uniform_real_distribution<double> distrib(-s, s);
  Eigen::MatrixXd A(rows, cols);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      A(i, j) = distrib(*rng);
    }
  }
  return A;
}

bool eq(const Eigen::MatrixXd &a, const Eigen::MatrixXd &b, double tol) {
  int rows = a.rows();
  int cols = b.cols();
  if (rows == b.rows() && cols == b.cols()) {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        if (std::abs(double(a(i, j)) - double(b(i, j))) > tol) {
          return false;
        }
        return true;
      }
    }
  }
  return false;
}


ABPair compress(const ABPair &pair) {
  auto Q = orthonormalBasis(pair.A);
  Eigen::MatrixXd R = Q.transpose()*pair.A;
  Eigen::VectorXd QtB = Q.transpose()*pair.B;
  return ABPair{R, QtB};
}

}
} /* namespace sail */
