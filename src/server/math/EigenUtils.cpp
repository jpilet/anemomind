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


ABPair compress(const Eigen::MatrixXd &A, const Eigen::MatrixXd &B) {
  auto Q = orthonormalBasis(A);
  Eigen::MatrixXd R = Q.transpose()*A;
  Eigen::MatrixXd QtB = Q.transpose()*B;
  return ABPair(R, QtB);
}

namespace {
  bool empty(const Eigen::MatrixXd &A) {
    return A.rows() == 0 && A.cols() == 0;
  }

  Eigen::MatrixXd addPossiblyEmpty(const Eigen::MatrixXd &A,
                                   const Eigen::MatrixXd &B) {
    if (empty(A)) {
      return B;
    } else if (empty(B)) {
      return A;
    } else {
      return A + B;
    }
  }
}

bool ABPair::empty() const {
  return EigenUtils::empty(A) && EigenUtils::empty(B);
}

ABPair ABPair::operator+(const ABPair &other) const {
  return ABPair(addPossiblyEmpty(A, other.A),
                addPossiblyEmpty(B, other.B));
}

ABPair ABPair::operator*(double factor) const {
  return ABPair(factor*A, factor*B);
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


Eigen::MatrixXd hcat(const Eigen::MatrixXd &A, const Eigen::VectorXd &B) {
  CHECK(A.rows() == B.rows());
  Eigen::MatrixXd AB(A.rows(), A.cols() + 1);
  AB << A, B;
  return AB;
}

ABPair makeNormalEqs(Eigen::MatrixXd A, Eigen::MatrixXd B) {
  return ABPair(A.transpose()*A, A.transpose()*B);
}

}
} /* namespace sail */
