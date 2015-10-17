/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LinearOptCalib.h"
#include <server/math/nonlinear/DataFit.h>

namespace sail {
namespace LinearOptCalib {

using namespace Eigen;

template <typename T>
bool hasEvenRows(const T &x) {
  return x.rows() % 2 == 0;
}

MatrixXd orthonormalBasis(const MatrixXd &x) {
  return MatrixXd(
      HouseholderQR<MatrixXd>(x)
        .householderQ()*MatrixXd::Identity(
            x.rows(), x.cols()));
}

MatrixXd assembleSpans(const MatrixXd &A, Array<Spani> spans) {
  int dstSize = spans.reduce<int>(0, [](int x, Spani y) {return x + y.width();});
  int cols = A.cols();
  MatrixXd result(dstSize, cols);
  int from = 0;
  for (auto span: spans) {
    auto span2 = 2*span;
    int to = from + span2.width();
    result.block(from, 0, to-from, cols)
        = A.block(span2.minv(), 0, span2.width(), cols);
  }
  return result;
}

MatrixXd makeLhs(const MatrixXd &A, Array<Spani> spans) {
  return orthonormalBasis(assembleSpans(A, spans));
}

Results optimize(
    const MatrixXd &A, const VectorXd &B,
    Array<Spani> spans,
    const Settings &settings) {
  assert(hasEvenRows(A));
  assert(hasEvenRows(B));
  assert(A.rows() == B.rows());

}


}
}
