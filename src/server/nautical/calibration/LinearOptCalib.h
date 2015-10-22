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


// The Eigen::SparseVector class is unnecessarily complicated for our limited use.
// In particular, initialization is awkward. So make our own stuff instead.
class SparseVector {
 public:
  struct Entry {
   int index;
   double value;
   bool operator<(const Entry &other) const {
     return index < other.index;
   }
  };

  SparseVector(int dim, Array<Entry> entries);
  SparseVector() : _dim(0) {}

  static SparseVector zeros(int dim);

  const Array<Entry> &entries() const {
    return _entries;
  }

  int dim() const {
    return _dim;
  }

  int nnz() const {
    return _entries.size();
  }
 private:
  int _dim;
  Array<Entry> _entries;
};

SparseVector operator+(const SparseVector &a, const SparseVector &b);
SparseVector operator-(const SparseVector &a, const SparseVector &b);
SparseVector operator-(const SparseVector &a);
SparseVector operator*(double factor, const SparseVector &x);
double dot(const SparseVector &a, const SparseVector &b);
double squaredNorm(const SparseVector &x);
double norm(const SparseVector &x);
SparseVector normalize(const SparseVector &x);
SparseVector projectOnNormalized(const SparseVector &a, const SparseVector &bHat);
SparseVector project(const SparseVector &a, const SparseVector &b);


Eigen::MatrixXd orthonormalBasis(const Eigen::MatrixXd &x);
Eigen::MatrixXd makeLhs(const Eigen::MatrixXd &A, Array<Spani> spans);
Eigen::SparseMatrix<double> makeRhs(const Eigen::VectorXd &B, Array<Spani> spans);
Eigen::MatrixXd makeParameterizedApparentFlowMatrix(const Eigen::MatrixXd &A, Array<Spani> spans);
Array<Spani> makeOverlappingSpans(int dataSize, int spanSize, double relativeStep = 0.5);



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
