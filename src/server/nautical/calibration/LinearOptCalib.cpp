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
    std::vector<DataFit::Triplet> *dst, Eigen::VectorXd *Bopt,
    CoordIndexer bRows) {
  double c = 1.0/sqrt(rows.count());
  double bdot = 0.0;
  auto col = colBlock[i];
  bool orthonormalize = Bopt != nullptr;
  for (auto index: rows.coordinateSpan()) {
    auto row = rows.span(index)[i];
    dst->push_back(DataFit::Triplet(row, col, c));
    if (orthonormalize) {
      bdot += c*(*Bopt)(bRows.span(index)[i]);
    }
  }
  if (orthonormalize) {
    for (auto index: rows.coordinateSpan()) {
      auto row = bRows.span(index)[i];
      (*Bopt)(row) -= bdot*c;
    }
  }
}

void addFlowColumns(CoordIndexer rows, Spani colBlock,
  std::vector<DataFit::Triplet> *dst, Eigen::VectorXd *Bopt, CoordIndexer bRows) {
  assert(rows.dim() == colBlock.width());
  assert(0 <= rows.from());
  assert(Bopt == nullptr || bRows.to() <= Bopt->size());
  for (auto i: colBlock.indices()) {
    addFlowColumn(rows, colBlock, i, dst, Bopt, bRows);
  }
}

void addFlowColumns(Array<CoordIndexer> rowIndexers, CoordIndexer cols,
  std::vector<Triplet> *dst, Eigen::VectorXd *Bopt, Array<CoordIndexer> bRows) {
  for (auto i: Spani::indicesOf(rowIndexers)) {
    addFlowColumns(rowIndexers[i], cols.span(i), dst, Bopt, bRows[i]);
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

Eigen::VectorXd assembleVector(Arrayi indexMap, Eigen::VectorXd src) {
  Eigen::VectorXd dst(indexMap.size());
  for (auto i: Spani::indicesOf(indexMap)) {
    dst[i] = src[indexMap[i]];
  }
  return dst;
}

Eigen::MatrixXd assembleMatrix(Arrayi indexMap, Eigen::MatrixXd src) {
  Eigen::MatrixXd dst(indexMap.size(), src.cols());
  for (auto i: Spani::indicesOf(indexMap)) {
    int srcRow = indexMap[i];
    for (int j = 0; j < src.cols(); j++) {
      dst(i, j) = src(srcRow, j);
    }
  }
  return dst;
}

void insertDenseVectorIntoSparseMatrix(double factor, const Eigen::VectorXd &src,
  Spani dstRowSpan, int dstCol, std::vector<DataFit::Triplet> *dst) {
  for (auto i: dstRowSpan.indices()) {
    dst->push_back(DataFit::Triplet(dstRowSpan[i], dstCol, factor*src[i]));
  }
}

void insertDenseMatrixIntoSparseMatrix(const Eigen::MatrixXd &src,
  Spani dstRowSpan, Spani dstColSpan, std::vector<DataFit::Triplet> *dst) {
  for (auto i: dstRowSpan.indices()) {
    int dstRow = dstRowSpan[i];
    for (auto j: dstColSpan.indices()) {
      dst->push_back(DataFit::Triplet(dstRow, dstColSpan[j], src(i, j)));
    }
  }
}


/*Eigen::VectorXd solveBasic(const Eigen::SparseMatrix &Qab) {
  Eigen::SparseMatrix<double> QtQ = Qab.transpose()*Qab;
  Eigen::SelfAdjointEigenSolver<SparseMatrix<double> >
}*/

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

void addOrthoGpsAndFlowData(Eigen::VectorXd assembledB,
    Array<CoordIndexer> rowIndexers,
    CoordIndexer fullFlowCols, std::vector<Triplet> *fullElements,
    CoordIndexer::Factory rows,
    CoordIndexer fullGpsCols) {
  Eigen::VectorXd tempB = -assembledB;
  addFlowColumns(rowIndexers, fullFlowCols,
    fullElements, &tempB, rowIndexers);
  insertDenseVectorIntoSparseMatrix((1.0/tempB.norm()), tempB,
      rows.span(), fullGpsCols.from(), fullElements);

}

Spani spanOf(Array<CoordIndexer> indexers) {
  return Spani(indexers.first().from(), indexers.last().to());
}

Problem makeProblem(const Eigen::MatrixXd &A, const Eigen::VectorXd &B,
    Array<Spani> spans) {
  Problem problem;

  CoordIndexer::Factory rows;
  CoordIndexer::Factory fullCols;
  CoordIndexer::Factory reducedCols;
  auto rowIndexers = spans.map2([&](Spani span) {
    return rows.make(span.width(), 2);
  });
  auto fullParamCols = fullCols.make(A.cols(), 1);
  auto fullGpsCols = fullCols.make(1, 1);
  auto fullFlowCols = fullCols.make(spans.size(), 2);
  auto reducedGpsCols = reducedCols.duplicate(fullGpsCols);
  auto reducedFlowCols = reducedCols.duplicate(fullFlowCols);

  auto indexMap = assembleIndexMap(rows.count(), rowIndexers, spans);

  problem.assembledA = assembleMatrix(indexMap, A);
  problem.assembledB = assembleVector(indexMap, B);


  auto orthoA = orthonormalBasis(problem.assembledA);
  std::vector<Triplet> fullElements, reducedElements;

  // Insert the parameter part first.
  insertDenseMatrixIntoSparseMatrix(orthoA, rows.span(), fullParamCols.elementSpan(), &fullElements);

  // For the full matrix. Here we do insert the orthonormal basis for everything
  addOrthoGpsAndFlowData(problem.assembledB, rowIndexers, fullFlowCols, &fullElements, rows, fullGpsCols);

  // For the reduced matrix. Here we don't orthonormalize it.
  // We need this matrix in order to recover the scale of the GPS column.
  addFlowColumns(rowIndexers, reducedFlowCols, &reducedElements, nullptr, rowIndexers);
  insertDenseVectorIntoSparseMatrix(1.0, -problem.assembledB,
      rows.span(), reducedGpsCols.from(), &reducedElements);


  SparseMatrix<double> fullProblemMatrix(rows.count(), fullCols.count());
  fullProblemMatrix.setFromTriplets(fullElements.begin(), fullElements.end());

  SparseMatrix<double> nonOrthoGpsAndFlowMatrix(rows.count(), reducedCols.count());
  nonOrthoGpsAndFlowMatrix.setFromTriplets(reducedElements.begin(), reducedElements.end());

  problem.orthoA = orthoA;
  problem.Rparam = orthoA.transpose()*problem.assembledA;
  problem.qaColSpan = fullParamCols.elementSpan();
  problem.Qab = fullProblemMatrix;
  problem.qbColSpan = Spani(fullParamCols.to(), fullCols.count());
  problem.nonOrthoGpsAndFlowMatrix = nonOrthoGpsAndFlowMatrix;
  problem.rowSpansToFit = rowIndexers.map2([&](const CoordIndexer &indexer) {
    return indexer.elementSpan();
  });

  /////// Add slack

  // Continue building QabWithSlack.
  auto slack1Rows = spans.map2([&](Spani span) {
    return rows.make(span.width(), 2);
  });
  auto slack2Rows = spans.map2([&](Spani span) {
    return rows.make(span.width(), 2);
  });

  insertDenseMatrixIntoSparseMatrix(orthoA, spanOf(slack1Rows), fullParamCols.elementSpan(), &fullElements);
  addOrthoGpsAndFlowData(problem.assembledB, slack2Rows, fullFlowCols, &fullElements, rows, fullGpsCols);

  SparseMatrix<double> QabWithSlack(rows.count(), fullCols.count());
  QabWithSlack.setFromTriplets(fullElements.begin(), fullElements.end());

  problem.QabWithSlack = QabWithSlack;

  return problem;
}


auto getSubVector(const Eigen::VectorXd &x, Spani span) -> decltype(x.block(0, 0, 1, 1)) {
  return x.block(span.minv(), 0, span.width(), 1);
}

Eigen::VectorXd setSpanToZero(Eigen::VectorXd X, Spani span) {
  int n = X.size();
  Eigen::VectorXd Y(n);
  for (int i = 0; i < n; i++) {
    Y[i] = (span.contains(i)? 0 : X[i]);
  }
  return Y;
}

Eigen::VectorXd Problem::computeParametersFromSolutionVector(const Eigen::VectorXd &X) const {
  auto orthoAParams = getSubVector(X, qaColSpan);
  auto orthoBParams = setSpanToZero(X, qbColSpan);
  ColPivHouseholderQR<MatrixXd> decomp(Rparam);
  auto scaledParams = decomp.solve(orthoAParams);
  Decomp Bd(nonOrthoGpsAndFlowMatrix.transpose()*nonOrthoGpsAndFlowMatrix);
  Eigen::VectorXd gpsAndFlowParams = Bd.solve(nonOrthoGpsAndFlowMatrix.transpose()*Qab*orthoBParams);
  double scale = gpsAndFlowParams(0);
  Eigen::VectorXd actualParams = (1.0/scale)*scaledParams;
  return actualParams;
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
