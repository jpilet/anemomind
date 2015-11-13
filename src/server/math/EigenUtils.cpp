/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/math/EigenUtils.h>
#include <server/math/Random.h>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <server/common/logging.h>

namespace sail {
namespace EigenUtils {

bool isOrthonormal(Eigen::MatrixXd Q, double tol) {
  Eigen::MatrixXd QtQ = Q.transpose()*Q;
  int n = QtQ.rows();
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      auto expected = (i == j? 1.0 : 0.0);
      if (std::abs(QtQ(i, j) - expected) > tol) {
        return false;
      }
    }
  }
  return true;
}

Eigen::MatrixXd projector(Eigen::MatrixXd A) {
  //return A*((A.transpose()*A).inverse()*A.transpose());
  Eigen::MatrixXd pInv = A.colPivHouseholderQr().solve(Eigen::MatrixXd::Identity(A.rows(), A.rows()));
  return A*pInv;
}

bool spanTheSameSubspace(Eigen::MatrixXd A, Eigen::MatrixXd B, double tol) {
  auto aProj = projector(A);
  auto bProj = projector(B);
  auto D = aProj - bProj;
  for (int i = 0; i < D.rows(); i++) {
    for (int j = 0; j < D.cols(); j++) {
      if (D(i, j) > tol) {
        return false;
      }
    }
  }
  return true;
}

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

Eigen::VectorXd smallestEigVec(const Eigen::MatrixXd &K) {
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(K);
  double minValue = std::numeric_limits<double>::infinity();
  int best = -1;
  for (int i = 0; i < solver.eigenvalues().size(); i++) {
    auto val = solver.eigenvalues()[i];
    if (val < minValue) {
      best = i;
      minValue = val;
    }
  }
  return solver.eigenvectors().block(0, best, K.rows(), 1);
}

// Through a change of basis, we
// make sure that the doniminator is constant w.r.t.
// to X if |X| is constant.
Eigen::VectorXd minimizeNormRatio(Eigen::MatrixXd A,
                                  Eigen::MatrixXd B) {
  Eigen::HouseholderQR<Eigen::MatrixXd> qr(B);
  Eigen::MatrixXd Q = qr.householderQ()*Eigen::MatrixXd::Identity(B.rows(), B.cols());
  Eigen::MatrixXd R = Q.transpose()*B;
  // People use to say it is a bad practice
  // to invert matrices. Wonder if there is a better way.
  // In matlab, I would do main = A/R
  Eigen::MatrixXd Rinv = R.inverse();
  Eigen::MatrixXd main = A*Rinv;
  Eigen::MatrixXd K = main.transpose()*main;
  return Rinv*smallestEigVec(K);
}



Eigen::MatrixXd hcat(const Eigen::MatrixXd &A, const Eigen::VectorXd &B) {
  CHECK(A.rows() == B.rows());
  Eigen::MatrixXd AB(A.rows(), A.cols() + 1);
  AB << A, B;
  return AB;
}

Eigen::VectorXd minimizeNormRatio(Eigen::MatrixXd A, Eigen::VectorXd B,
                                  Eigen::MatrixXd C, Eigen::VectorXd D) {
  auto Xh = minimizeNormRatio(hcat(A, B), hcat(C, D));
  int n = Xh.size() - 1;
  auto f = 1.0/Xh(n);
  Eigen::VectorXd X(n);
  for (int i = 0; i < n; i++) {
    X(i) = f*Xh(i);
  }
  return X;
}

}
} /* namespace sail */
