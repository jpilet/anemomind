/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/DataFit.h>
#include <server/common/math.h>

namespace sail {
namespace DataFit {

void add(int i, int j, double value, std::vector<Triplet> *dst) {
  assert(std::isfinite(value));
  dst->push_back(Triplet(i, j, value));
}

void makeEye(double weight, Spani rowSpan, Spani colSpan, std::vector<Triplet> *dst) {
  int n = std::min(rowSpan.width(), colSpan.width());
  for (int i = 0; i < n; i++) {
    add(rowSpan.minv() + i, colSpan.minv() + i, weight, dst);
  }
}

void makeReg(double weight, int order, CoordIndexer rowIndexer, CoordIndexer colIndexer,
    std::vector<Triplet> *dst) {
  assert(rowIndexer.dim() == colIndexer.dim());
  assert(rowIndexer.count() + order == colIndexer.count());
  Arrayd coefs = makeRegCoefs(order);
  for (auto i: rowIndexer.coordinateSpan()) {
    Spani rowSpan = rowIndexer.span(i);
    for (int j = 0; j < coefs.size(); j++) {
      Spani colSpan = colIndexer.span(i + j);
      makeEye(weight*coefs[j], rowSpan, colSpan, dst);
    }
  }
}

void insertDense(double scale, const Eigen::MatrixXd &src, Spani dstRows, Spani dstCols,
  std::vector<Triplet> *dst) {
  for (int i = 0; i < src.rows(); i++) {
    for (int j = 0; j < src.cols(); j++) {
      dst->push_back(Triplet(dstRows[i], dstCols[j], scale*src(i, j)));
    }
  }
}

Eigen::SparseMatrix<double> makeSparseMatrix(int rows, int cols,
  const std::vector<Triplet> &triplets) {
  Eigen::SparseMatrix<double> A(rows, cols);
  A.setFromTriplets(triplets.begin(), triplets.end());
  return A;
}


void VectorBuilder::add(Spani dstSpan, const Eigen::VectorXd &data) {
  _data.push_back(SubVector{dstSpan, data});
}

void VectorBuilder::add(Spani dstSpan, double value) {
  Eigen::VectorXd X = value*Eigen::VectorXd::Ones(dstSpan.width());
  add(dstSpan, X);
}

Eigen::VectorXd VectorBuilder::make(int rows) const {
  Eigen::VectorXd dst = Eigen::VectorXd::Zero(rows);
  Spani validSpan(0, rows);
  for (auto sv: _data) {
    auto span = sv.span;
    for (auto i: span.indices()) {
      auto srcIndex = i;
      auto dstIndex = span[i];
      if (validSpan.contains(dstIndex)) {
        dst(dstIndex) += sv.data(srcIndex);
      }
    }
  }
  return dst;
}



irls::WeightingStrategy::Ptr makeThresholdedInlierFilter(double threshold,
    CoordIndexer dataRows,
    CoordIndexer inlierSlackRows,
    CoordIndexer outlierSlackRows,
    CoordIndexer outlierPenaltyRows,

    CoordIndexer dataCols,
    CoordIndexer inlierCols,
    CoordIndexer outlierCols,

    std::vector<Triplet> *aDst, Eigen::VectorXd *bDst) {
  int count = dataRows.count();
  assert(dataRows.dim() == dataCols.dim());
  assert(inlierSlackRows.sameSizeAs(dataRows));
  assert(inlierSlackRows.sameSizeAs(inlierCols));
  assert(outlierSlackRows.sameSizeAs(outlierCols));
  assert(inlierSlackRows.count() == outlierSlackRows.count());
  assert(outlierSlackRows.dim() == 1);
  assert(outlierPenaltyRows.sameSizeAs(outlierSlackRows));

  Array<irls::BinaryConstraintGroup> groups(count);
  for (int i = 0; i < count; i++) {
    auto rowSpan = dataRows.span(i);
    auto inlierRowSpan = inlierSlackRows.span(i);
    for (auto j: inlierRowSpan) {
      (*bDst)(j) = 0.0;
    }
    makeEye(1.0, rowSpan, inlierCols.span(i), aDst);
    makeEye(1.0, inlierSlackRows.span(i), inlierCols.span(i), aDst);
    (*bDst)(outlierPenaltyRows[i]) = threshold;
    (*bDst)(outlierSlackRows[i]) = 0;
    groups[i] = irls::BinaryConstraintGroup(
        inlierSlackRows.span(i), outlierSlackRows.span(i));
  }
  makeEye(1.0, outlierPenaltyRows.elementSpan(), outlierCols.elementSpan(), aDst);
  makeEye(1.0, outlierSlackRows.elementSpan(), outlierCols.elementSpan(), aDst);
  return irls::WeightingStrategyArray<irls::BinaryConstraintGroup>::make(groups);
}


double calcSquaredResidual(const Arrayd &X) {
  double sum2 = 0.0;
  for (auto x: X) {
    sum2 += x*x;
  }
  return sum2;
}



Results assembleResults(irls::Results solution, CoordIndexer inlierSlack,
    CoordIndexer samples, double inlierThreshold) {
  auto t2 = sqr(inlierThreshold);
  MDArray2d out(samples.count(), samples.dim());
  for (int i = 0; i < samples.count(); i++) {
    for (int j = 0; j < samples.dim(); j++) {
      auto sp = samples.span(i);
      out(i, j) = solution.X[sp[j]];
    }
  }
  Arrayb inliers(inlierSlack.count());
  for (auto i: inlierSlack.coordinateSpan()) {
    auto sp = inlierSlack.span(i);
    double x = 0.0;
    for (auto j: sp) {
      x += sqr(solution.residuals(j));
    }
    inliers[i] = x < t2;
  }
  return Results{inliers, out};
}






}
}
