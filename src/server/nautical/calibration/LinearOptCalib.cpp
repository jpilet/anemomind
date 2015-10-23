/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LinearOptCalib.h"
#include <server/math/nonlinear/DataFit.h>
#include <server/common/string.h>
#include <server/common/ArrayBuilder.h>

namespace sail {
namespace LinearOptCalib {

using namespace Eigen;

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

MatrixXd makeParameterizedApparentFlowMatrix(const MatrixXd &A, Array<Spani> spans) {
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

Array<Spani> makeOverlappingSpans(int dataSize, int spanSize, double relativeStep) {
  LineKM spanStart(0, 1, 0, relativeStep*spanSize);
  auto lastSpanIndex = spanStart.solveWithEquality(dataSize - spanSize);
  int spanCount = int(floor(lastSpanIndex)) + 1;
  return Spani(0, spanCount).map<Spani>([&](int spanIndex) {
    int from = int(round(spanStart(spanIndex)));
    return Spani(from, from + spanSize);
  });
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

void addFlowColumn(const DataFit::CoordIndexer &rows, Spani colBlock,
    int i,
    std::vector<DataFit::Triplet> *dst, Eigen::VectorXd *Bopt) {
  double c = 1.0/sqrt(rows.count());
  double bdot = 0.0;
  auto col = colBlock[i];
  bool orthonormalize = Bopt != nullptr;
  for (auto index: rows.coordinateSpan()) {
    auto row = rows.span(index)[i];
    dst->push_back(DataFit::Triplet(row, col, c));
    if (orthonormalize) {
      bdot += c*(*Bopt)(row);
    }
  }
  if (orthonormalize) {
    for (auto index: rows.coordinateSpan()) {
      auto row = rows.span(index)[i];
      (*Bopt)(row) -= bdot*c;
    }
  }
}

void addFlowColumns(const DataFit::CoordIndexer &rows, Spani colBlock,
  std::vector<DataFit::Triplet> *dst, Eigen::VectorXd *Bopt) {
  assert(rows.dim() == colBlock.width());
  assert(0 <= rows.from());
  assert(Bopt == nullptr || rows.to() <= Bopt->size());
  for (auto i: colBlock.indices()) {
    addFlowColumn(rows, colBlock, i, dst, Bopt);
  }
}


int calcIdealCols(int spanCount) {
  return  1 + 2*spanCount;
}




/*Results optimize(
    const MatrixXd &A, const VectorXd &B,
    Array<Spani> spans0,
    const Settings &settings) {

  assert(hasEvenRows(A));
  assert(hasEvenRows(B));
  assert(A.rows() == B.rows());

}*/


}
}
