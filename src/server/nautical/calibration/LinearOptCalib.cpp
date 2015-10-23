/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LinearOptCalib.h"
#include <server/math/nonlinear/DataFit.h>
#include <server/common/string.h>
#include <server/common/ArrayBuilder.h>

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
  return Results{(maxEig - temp.X.norm())*(1.0/temp.X.norm())*temp.X, temp.iters};
}

} namespace LinearOptCalib {

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

void addFlowColumns(Array<CoordIndexer> rowIndexers, CoordIndexer cols,
  std::vector<Triplet> *dst, Eigen::VectorXd *Bopt) {
  for (auto i: Spani::indicesOf(rowIndexers)) {
    addFlowColumns(rowIndexers[i], cols.span(i), dst, Bopt);
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
  problem.assembledB = assembleMatrix(indexMap, B);


  auto orthoA = orthonormalBasis(problem.assembledA);
  std::vector<Triplet> fullElements, reducedElements;

  // Insert the parameter part first.
  insertDenseMatrixIntoSparseMatrix(orthoA, rows.span(), fullParamCols.elementSpan(), &fullElements);

  // For the full matrix. Here we do insert the orthonormal basis for everything
  Eigen::VectorXd tempB = -problem.assembledB;
  addFlowColumns(rowIndexers, fullFlowCols,
    &fullElements, &tempB);
  insertDenseVectorIntoSparseMatrix((1.0/tempB.norm()), tempB,
      rows.span(), fullGpsCols.from(), &fullElements);

  // For the reduced matrix. Here we don't orthonormalize it.
  // We need this matrix in order to recover the scale of the GPS column.
  addFlowColumns(rowIndexers, reducedFlowCols, &reducedElements, nullptr);
  insertDenseVectorIntoSparseMatrix(1.0, -problem.assembledB,
      rows.span(), reducedGpsCols.from(), &reducedElements);


  SparseMatrix<double> fullProblemMatrix(rows.count(), fullCols.count());
  fullProblemMatrix.setFromTriplets(fullElements.begin(), fullElements.end());

  SparseMatrix<double> nonOrthoGpsAndFlowMatrix(rows.count(), reducedCols.count());
  nonOrthoGpsAndFlowMatrix.setFromTriplets(reducedElements.begin(), reducedElements.end());

  problem.Rparam = orthoA.transpose()*problem.assembledA;
  problem.qaColSpan = fullParamCols.elementSpan();
  problem.Qab = fullProblemMatrix;
  problem.qbColSpan = Spani(fullParamCols.to(), fullCols.count());
  problem.nonOrthoGpsAndFlowMatrix = nonOrthoGpsAndFlowMatrix;
  problem.rowSpansToFit = rowIndexers.map2([&](const CoordIndexer &indexer) {
    return indexer.elementSpan();
  });

  // Continue building L.
  //CoordIndexer::Factory qRows = rows;
  //auto

  return problem;
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
