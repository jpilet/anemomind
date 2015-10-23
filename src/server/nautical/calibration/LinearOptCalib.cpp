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
using namespace DataFit;

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
    std::vector<Triplet> *dst) {
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


Arrayi assembleIndexMap(int dstElemCount,
    Array<DataFit::CoordIndexer> dstIndexers,
  Array<Spani> srcSpans) {
  Arrayi dst(dstElemCount);
  for (auto i: Spani::indicesOf(dstIndexers)) {
    auto di = dstIndexers[i];
    auto dstInds = di.elementSpan();
    auto srcInds = di.dim()*srcSpans[i];
    assert(srcInds.width() == dstInds.width());
    for (auto i: dstInds.indices()) {
      dst[dstInds[i]] = srcInds[i];
    }
  }
  return dst;
}

Eigen::VectorXd copyAndPasteTogetherVector(
    int dstElemCount,
    Array<DataFit::CoordIndexer> dstIndexers,
    Array<Spani> srcSpans,
    const Eigen::VectorXd &src) {
  Arrayi indexMap = assembleIndexMap(dstElemCount, dstIndexers, srcSpans);
  Eigen::VectorXd dst(dstElemCount);
  for (auto i: Spani::indicesOf(indexMap)) {
    dst[i] = src[indexMap[i]];
  }
  return dst;
}

void insertVectorIntoSparseMatrix(double factor, const Eigen::VectorXd &src,
  Spani dstRowSpan, int dstCol, std::vector<DataFit::Triplet> *dst) {
  for (auto i: dstRowSpan.indices()) {
    dst->push_back(DataFit::Triplet(dstRowSpan[i], dstCol, factor*src[i]));
  }
}

/*
struct Problem {
  Array<Spani> rowSpansToFit;
  Spani paramColSpan;
  Spani gpsAndFlowColSpan;
  Eigen::MatrixXd Rparam;
  Eigen::SparseMatrix<double> fullProblemMatrix;
  Eigen::SparseMatrix<double> orthoGpsAndFlowMatrix, nonOrthoGpsAndFlowMatrix;
};
*/

Problem makeProblem(const Eigen::MatrixXd &A, const Eigen::VectorXd &B,
    Array<Spani> spans) {
  CoordIndexer::Factory rows;
  CoordIndexer::Factory fullCols;
  CoordIndexer::Factory reducedCols;
  auto rowIndexers = spans.map2([&](Spani span) {
    return rows.make(span.width(), 2);
  });
  auto fullParamCols = fullCols.make(4, 1);
  auto fullGpsCols = fullCols.make(1, 1);
  auto fullFlowCols = fullCols.make(spans.size(), 1);
  auto reducedGpsCols = reducedCols.duplicate(fullGpsCols);
  auto reducedFlowCols = reducedCols.duplicate(fullFlowCols);


  //auto BAssembled = copyAndPasteTogetherVector(rows.count(), B, spans);

  std::vector<Triplet> fullElements;
  //addDenseMatrix(full);
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
