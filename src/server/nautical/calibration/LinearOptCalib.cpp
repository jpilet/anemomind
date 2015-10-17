/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LinearOptCalib.h"
#include <server/math/nonlinear/DataFit.h>

namespace sail {
namespace LinearOptCalib {

using namespace Eigen;

bool isEven(int x) {
  return x % 2 == 0;
}

template <typename T>
bool hasEvenRows(const T &x) {
  return isEven(x.rows());
}

MatrixXd orthonormalBasis(const MatrixXd &x) {
  return MatrixXd(
      HouseholderQR<MatrixXd>(x)
        .householderQ()*MatrixXd::Identity(
            x.rows(), x.cols()));
}

int countFlowEqs(Array<Spani> spans) {
  return spans.reduce<int>(0, [](int x, Spani y) {return x + y.width();});
}

MatrixXd assembleSpans(const MatrixXd &A, Array<Spani> spans) {
  int cols = A.cols();
  MatrixXd result(2*countFlowEqs(spans), cols);
  int from = 0;
  for (auto span: spans) {
    auto span2 = 2*span;
    int to = from + span2.width();
    result.block(from, 0, to-from, cols)
        = A.block(span2.minv(), 0, span2.width(), cols);
    from = to;
  }
  assert(from == result.rows());
  return result;
}


MatrixXd makeLhs(const MatrixXd &A, Array<Spani> spans) {
  return orthonormalBasis(assembleSpans(A, spans));
}

void addSpanWithConstantFlow(Spani srcSpan, Spani dstSpan, int spanIndex, const VectorXd &B,
    std::vector<Triplet<double> > *dst) {
  assert(isEven(srcSpan.width()));
  assert(srcSpan.width() == dstSpan.width());
  int colOffset = 1 + 2*spanIndex;
  for (auto i: srcSpan.indices()) {
    int dstRow = dstSpan[i];
    dst->push_back(Eigen::Triplet<double>(dstRow, 0, B(srcSpan[i])));
    dst->push_back(Eigen::Triplet<double>(dstRow, colOffset + (isEven(i)? 0 : 1), 1.0));
  }
}

int calcIdealCols(int spanCount) {
  return  1 + 2*spanCount;
}

/*
Eigen::SparseMatrix<double> assembleSpansWithConstantFlow(
    Eigen::VectorXd B, Array<Spani> spans) {
  std::sort(spans.begin(), spans.end());
  int n = countFlowEqs(spans);
  int idealCols = calcIdealCols(spans.size());
  int rows = 2*n;
  assert(idealCols <= rows);
  SparseMatrix<double> dst(rows, idealCols);
  std::vector<Triplet<double> > triplets;
  int from = 0;
  for (int i = 0; i < spans.size(); i++) {
    auto span2 = 2*spans[i];
    auto to = from + span2.width();
    addSpanWithConstantFlow(span2, Spani(from, to), i, B, &triplets);
    from = to;
  }
  assert(from == dst.rows());
  dst.setFromTriplets(triplets.begin(), triplets.end());
  return dst;
}


  SparseMatrix<double> orthonormalBasis(Eigen::SparseMatrix<double> X, int idealCols) {
  SparseQR<SparseMatrix<double>, Eigen::COLAMDOrdering<int> > qr(X);
  SparseMatrix<double> q; //(X.rows(), X.cols());
  SparseMatrix<double> id(X.rows(), idealCols);
  id.setIdentity();
  q = (qr.matrixQ()*id);
  return q;
}

Eigen::SparseMatrix<double> makeRhs(const VectorXd &B, Array<Spani> spans) {
  auto assembled = assembleSpansWithConstantFlow(B, spans);
  return orthonormalBasis(assembled);
}*/



class ByWidth {
 public:
  bool operator() (const Spani &a, const Spani &b) const {
    return a.width() < b.width();
  }
};

Results optimize(
    const MatrixXd &A, const VectorXd &B,
    Array<Spani> spans0,
    const Settings &settings) {

  // Sort, so that the orthonormal basis becomes as sparse as possible.
  Array<Spani> spans = spans0.dup();
  std::sort(spans.begin(), spans.end(), ByWidth());

  assert(hasEvenRows(A));
  assert(hasEvenRows(B));
  assert(A.rows() == B.rows());

}


}
}
