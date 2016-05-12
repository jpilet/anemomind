/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/irls.h>
#include <cmath>
#include <server/common/math.h>
#include <server/common/LineKM.h>
#include <Eigen/SparseCholesky>
#include <server/common/string.h>
#include <server/common/ScopedLog.h>
#include <server/common/ArrayIO.h>
#include <server/common/Functional.h>
#include <server/math/lapack/DgbsvWrapper.h>

namespace sail {
namespace irls {

typedef Eigen::Triplet<double> Triplet;

bool isSane(int n, const double *X) {
  int counter = 0;
  int maxWarnings = 3;
  for (int i = 0; i < n; i++) {
    if (!std::isfinite(X[i])) {
      if (counter < maxWarnings) {
        LOG(WARNING) << "Element " << i << " is bad.";
      }
      counter++;
    }
  }
  if (counter > maxWarnings) {
    LOG(WARNING) << " ...and " << counter - maxWarnings << " other elements.";
  }
  return counter == 0;
}

bool isSane(Arrayd X) {
  return isSane(X.size(), X.ptr());
}

bool isSane(const Eigen::VectorXd &X) {
  return isSane(X.size(), X.data());
}

Arrayd toArray(Eigen::VectorXd &v) {
  return Arrayd(v.size(), v.data());
}


struct Residual {
 Spani span;
 double value;

 bool operator< (const Residual &other) const {
   return value < other.value;
 }
};

double calcResidualForSpan(Spani span, const Arrayd &residualVector) {
  if (span.width() == 1) {
    return std::abs(residualVector[span.minv()]);
  } else {
    double sum = 0.0;
    for (auto i: span) {
      sum += sqr(residualVector[i]);
    }
    return (sum <= 0? 0 : sqrt(sum));
  }
}

Array<Residual> buildResidualsPerConstraint(Array<Spani> allConstraintGroups,
  const Arrayd &residualVector) {
  int n = allConstraintGroups.size();
  Array<Residual> dst(n);
  for (int i = 0; i < n; i++) {
    auto span = allConstraintGroups[i];
    dst[i] = Residual{span, calcResidualForSpan(span, residualVector)};
  }
  std::sort(dst.begin(), dst.end());
  return dst;
}


bool allPositive(Arrayd X) {
  return X.all([](double x) {return x > 0;});
}


double getSqrtResidual(const Arrayd &residuals, int index) {
  double marg = 1.0e-12; // <- for well-posedness.
  return sqrt(residuals[index] + marg);
}

typedef Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> > Decomp;


Arrayd distributeWeights(Arrayd residuals, double avgWeight) {
  int n = residuals.size();
  int paramCount = n-1;
  int elemCount = 2*paramCount;
  Array<Triplet> triplets(elemCount);
  for (int i = 0; i < paramCount; i++) {
    int offset = 2*i;
    double r0 = getSqrtResidual(residuals, i);
    double r1 = getSqrtResidual(residuals, i+1);
    triplets[offset + 0] = Triplet(i+0, i, r0);
    triplets[offset + 1] = Triplet(i+1, i, -r1);
  }
  Eigen::VectorXd B(n);
  for (int i = 0; i < n; i++) {
    B[i] = -getSqrtResidual(residuals, i)*avgWeight;
  }
  Eigen::SparseMatrix<double> A(n, paramCount);
  A.setFromTriplets(triplets.begin(), triplets.end());
  Decomp decomp(A.transpose()*A);
  Eigen::VectorXd result = decomp.solve(A.transpose()*B);
  Arrayd weights = Arrayd::fill(n, avgWeight);
  for (int i = 0; i < paramCount; i++) {
    weights[i] += result[i];
    weights[i+1] -= result[i];
  }
  return weights;
}

Arrayd threshold(Array<Residual> residuals, int activeCount, double minResidual) {
  int n = residuals.size();
  Arrayd Y(n);
  Y.sliceTo(activeCount).setTo(std::max(minResidual, residuals[activeCount-1].value));
  for (int i = activeCount; i < n; i++) {
    Y[i] = std::max(minResidual, residuals[i].value);
  }
  return Y;
}

int countCoefs(Array<Residual> residuals) {
  int counter = 0;
  for (auto r: residuals) {
    counter += r.span.width();
  }
  return counter;
}


DiagMat makeWeightMatrixSub(int aRows,
    Array<Array<Residual> > residualsPerGroup, Array<Arrayd> weightsPerGroup) {

  int n = residualsPerGroup.size();

  DiagMat W(aRows);
  W.setIdentity();
  auto &v = W.diagonal();
  for (int i = 0; i < n; i++) {
    auto residuals = residualsPerGroup[i];
    auto weights = weightsPerGroup[i];
    for (int i = 0; i < residuals.size(); i++) {
      auto span = residuals[i].span;
      auto w = weights[i];
      for (auto i: span) {
        v(i) = w;
      }
    }
  }
  return W;
}

QuadCompiler::WeightsAndOffset QuadCompiler::makeWeightsAndOffset() const {
  int n = _quads.size();
  DiagMat W(n);
  Eigen::VectorXd offsets(n);
  W.setIdentity();
  auto &v = W.diagonal();
  for (int i = 0; i < n; i++) {
    const MajQuad &q = _quads[i];
    if (q.defined()) {
      auto line = q.factor();
      v(i) = line.getK();
      offsets(i) = line.getM();
    } else {
      v(i) = 1.0;
      offsets(i) = 0.0;
    }
  }
  assert(isSane(v));
  assert(isSane(offsets));
  return WeightsAndOffset{W, offsets};
}

void ConstraintGroup::apply(
    double constraintWeight, const Arrayd &residuals, QuadCompiler *dst) {
  Array<Residual> residualsPerConstraint = buildResidualsPerConstraint(_spans,
    residuals);

  auto thresholdedResiduals = threshold(residualsPerConstraint, _activeCount, _minResidual);
  Arrayd weights = distributeWeights(
      thresholdedResiduals,
      constraintWeight);
  int n = weights.size();
  assert(n == _spans.size());
  assert(n == residualsPerConstraint.size());
  for (int i = 0; i < n; i++) {
    double w = weights[i];
    const Spani &span = residualsPerConstraint[i].span;
    for (auto j : span) {
      dst->setWeight(j, w);
    }
  }
}

void BinaryConstraintGroup::apply(double constraintWeight,
    const Arrayd &residuals, QuadCompiler *dst) {
  auto totalCstWeight = 2.0*constraintWeight;
  auto ra = calcResidualForSpan(_a, residuals);
  auto rb = calcResidualForSpan(_b, residuals);
  auto aw = toFinite(rb/(ra + rb), 0.5);
  auto bw = 1.0 - aw;
  auto awc = totalCstWeight*aw;
  auto bwc = totalCstWeight*bw;
  for (auto i : _a) {
    dst->setWeight(i, awc);
  }
  for (auto i : _b) {
    dst->setWeight(i, bwc);
  }
}

WeightingStrategy::Ptr ConstraintGroup::make(Array<Spani> spans, int activeCount) {
  return WeightingStrategy::Ptr(new ConstraintGroup(spans, activeCount));
}

WeightingStrategy::Ptr Constant::make(Arrayi inds, MajQuad quad) {
  return WeightingStrategyArray<Constant>::make(toArray(map(inds,
      [=](int index) {
    return Constant(index, quad);
  })));
}

WeightingStrategy::Ptr NonNegativeConstraint::make(int index) {
  return WeightingStrategy::Ptr(new NonNegativeConstraint(index));
}

WeightingStrategy::Ptr NonNegativeConstraint::make(Arrayi inds) {
  return WeightingStrategyArray<NonNegativeConstraint>::make(toArray(map(inds,
      [=](int index) {
    return NonNegativeConstraint(index);
  })));
}

WeightingStrategy::Ptr Constant::make(int index, MajQuad quad) {
  return WeightingStrategy::Ptr(new Constant(index, quad));
}


Eigen::VectorXd product(const Eigen::SparseMatrix<double> &A, const Eigen::VectorXd &X) {
  Eigen::VectorXd Y = Eigen::VectorXd::Zero(A.rows());
  Y += A*X;
  return Y;
}


namespace {


  std::function<double(int)> makeIterationWeighter(const Settings &settings) {
    LineKM logWeights(0, settings.iters-1,
            log(settings.initialWeight), log(settings.finalWeight));
    return [=](int i) {
      return exp(logWeights(i));
    };
  }

  QuadCompiler fillQuadCompiler(
      double constraintWeight,
      Eigen::VectorXd &residuals,
      const Array<WeightingStrategy::Ptr> &strategies) {
    QuadCompiler weighter(residuals.size());
    auto residualArray = toArray(residuals);
    assert(isSane(residualArray));
    for (auto strategy: strategies) {
      CHECK(bool(strategy));
      strategy->apply(constraintWeight, residualArray, &weighter);
    }
    return weighter;
  }

  Eigen::VectorXd initializeResiduals(int rows) {
    return Eigen::VectorXd::Constant(rows, 1.0);
  }
}

Results solve(const Eigen::SparseMatrix<double> &A,
    const Eigen::VectorXd &B,
    Array<std::shared_ptr<WeightingStrategy> > strategies,
    Settings settings) {

  ENTERSCOPE("irls::Solve");
  int rows = A.rows();
  assert(rows == B.rows());
  auto residuals = initializeResiduals(rows);
  Eigen::VectorXd X;
  auto iterWeighter = makeIterationWeighter(settings);

  for (int i = 0; i < settings.iters; i++) {
    double constraintWeight = iterWeighter(i);

    SCOPEDMESSAGE(INFO, stringFormat("  Iteration %d/%d with weight %.3g",
        i+1, settings.iters, constraintWeight));

    auto weighter = fillQuadCompiler(constraintWeight, residuals, strategies);
    auto wk = weighter.makeWeightsAndOffset();
    Eigen::SparseMatrix<double> WA = wk.weights*A;
    Eigen::VectorXd WB = wk.weights*B - wk.offset;
    assert(isSane(wk.offset));
    assert(isSane(B));
    assert(isSane(WB));
    X = Decomp(WA.transpose()*WA).solve(WA.transpose()*WB);
    assert(isSane(X));
    residuals = product(A, X) - B;
    assert(isSane(residuals));
  }
  return Results{X, residuals};
}

int DenseBlock::minDiagWidth() const {
  return lhsCols() - 1;
}

namespace {
  bool validBlocks(int aRows, int aCols,
      const sail::Array<DenseBlock::Ptr> &blocks) {
    if (blocks.empty()) {
      LOG(ERROR) << "Empty list of blocks";
      return false;
    }

    for (auto blk: blocks) {
      if (!blk) {
        LOG(ERROR) << "One of the blocks is a nullptr";
        return false;
      }

      if (!(blk->requiredRows() <= aRows)) {
        LOG(ERROR) << "One of the blocks requires more rows than specified";
        return false;
      }

      if (!(blk->requiredCols() <= aCols)) {
        LOG(ERROR) << "One of the blocks requires more cols than specified";
      }
    }
    return true;
  }

  struct BParams {
    BParams(const Array<DenseBlock::Ptr> &blocks);
    int Xcols;
    int diagWidth;

    bool valid() const {return 0 < Xcols;}
  };

  BParams::BParams(const Array<DenseBlock::Ptr> &blocks) {
    Xcols = blocks[0]->rhsCols();
    diagWidth = blocks[0]->minDiagWidth();
    for (auto blk: blocks.sliceFrom(1)) {
      if (blk->rhsCols() != Xcols) {
        Xcols = 0;
        LOG(ERROR) << "Inconsistent required numbers of cols in X";
        break;
      }
      diagWidth = std::max(diagWidth, blk->minDiagWidth());
    }
  }

  bool validOffset(int cols, const Eigen::VectorXd &X) {
    if (cols == 1) {
      return true;
    }

    for (int i = 0; i < X.rows(); i++) {
      if (std::abs(X(i)) > 1.0e-6) {
        LOG(ERROR) << "The element " << i << " of offset is "
            << X(i) << " which is greater than 0 and cannot be used "
            " with multi-column solutions";
        return false;
      }
    }
    return true;
  }

  Eigen::Map<Eigen::MatrixXd> toEigen(MDArray2d X) {
    return Eigen::Map<Eigen::MatrixXd>(X.ptr(), X.rows(), X.cols());
  }

  ResultsMat solveBandedSub(const BParams &params, int rows, int cols,
      const Array<DenseBlock::Ptr> &blocks,
      const Array<std::shared_ptr<WeightingStrategy> > &strategies,
      const Settings &settings) {

    ENTERSCOPE("irls::solveBanded");

    SCOPEDMESSAGE(INFO, stringFormat("Extra diagonal width: %d", params.diagWidth));
    SCOPEDMESSAGE(INFO, stringFormat("Number of cols in X : %d", params.Xcols));

    auto residuals = initializeResiduals(rows);
    auto iterWeighter = makeIterationWeighter(settings);

    BandMatrix<double> AtA(cols, cols, params.diagWidth, params.diagWidth);
    MDArray2d AtB(cols, params.Xcols);
    Eigen::MatrixXd X;
    for (int i = 0; i < settings.iters; i++) {
      AtA.setAll(0.0);
      AtB.setAll(0.0);

      double constraintWeight = iterWeighter(i);

      SCOPEDMESSAGE(INFO, stringFormat("  Iteration %d/%d with weight %.3g",
          i+1, settings.iters, constraintWeight));

      auto weighter = fillQuadCompiler(constraintWeight, residuals, strategies);
      auto wk = weighter.makeWeightsAndOffset();
      if (!validOffset(params.Xcols, wk.offset)) {
        LOG(ERROR) << "Invalid offset";
        return ResultsMat();
      }

      Eigen::VectorXd weights(wk.weights.diagonal());

      for (auto block: blocks) {
        block->accumulateWeighted(weights, &AtA, &AtB);
      }

      if (!easyDgbsvInPlace(&AtA, &AtB)) {
        LOG(ERROR) << "Failed to perform dgbsv in irls::solveBanded";
        return ResultsMat();
      }
      X = toEigen(AtB);

      residuals = Eigen::VectorXd::Zero(rows);
      for (auto block: blocks) {
        block->eval(X, &residuals);
      }
      assert(isSane(residuals));
    }
    return ResultsMat{X, residuals};
  }
}

Eigen::Map<Eigen::MatrixXd,
  Eigen::Unaligned,
    Eigen::Stride<Eigen::Dynamic, 1> > bandMatrixView(
        BandMatrix<double> *src, int N, int offset) {
  return Eigen::Map<Eigen::MatrixXd,
          Eigen::Unaligned,
           Eigen::Stride<Eigen::Dynamic, 1> >(src->ptr(offset, offset), N, N,
               Eigen::Stride<Eigen::Dynamic, 1>(src->horizontalStride(), 1));
}

Eigen::Map<Eigen::MatrixXd,
    Eigen::Unaligned, Eigen::OuterStride<> > arrayView(
        MDArray2d *X, int rows, int cols, int rowOffsest) {
  assert(X->cols() == cols);
  return Eigen::Map<Eigen::MatrixXd,
      Eigen::Unaligned, Eigen::OuterStride<> >(X->getPtrAt(rowOffsest, 0), rows, cols,
          Eigen::OuterStride<>(X->getStep()));
}

ResultsMat solveBanded(int rows, int cols,
    const sail::Array<DenseBlock::Ptr> &blocks,
    Array<std::shared_ptr<WeightingStrategy> > strategies,
    Settings settings) {
  if (!validBlocks(rows, cols, blocks)) {
    LOG(ERROR) << "Failed to solve irls";
    return ResultsMat();
  }

  BParams params(blocks);
  if (!params.valid()) {
    LOG(ERROR) << "Failed to solve irls";
    return ResultsMat();
  }

  return solveBandedSub(params, rows, cols, blocks, strategies, settings);
}


}
}
