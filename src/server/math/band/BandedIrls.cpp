/*
 * BandedIrls.cpp
 *
 *  Created on: 30 Mar 2017
 *      Author: jonas
 */

#include "BandedIrls.h"
#include <server/math/band/BandMatrix.h>
#include <server/common/logging.h>
#include <server/common/ArrayIO.h>

namespace sail {
namespace BandedIrls {


BandProblem::BandProblem(int lhsDims,
    int rhsDims, int maxDiagWidth,
    double initialDiagElement) {
  _lhs = SymmetricBandMatrixL<double>::zero(lhsDims, maxDiagWidth);
  _rhs = MDArray2d(lhsDims, rhsDims);
  _rhs.setAll(0.0);
  for (int i = 0; i < lhsDims; i++) {
    _lhs.atUnsafe(i, i) = initialDiagElement;
  }
}


MDArray2d BandProblem::solveAndInvalidate() {
  CHECK(!empty());
  auto success = Pbsv<double>::apply(&_lhs, &_rhs);
  auto result = success? _rhs : MDArray2d();
  _lhs = SymmetricBandMatrixL<double>();
  _rhs = MDArray2d();
  return result;
}

struct ProblemSummary {
  int dimension = 0;
  int diagonalWidth = 0;
  int rightHandSideDimension = 0;
  ProblemSummary(const Array<Cost::Ptr>& costs) {
    rightHandSideDimension = costs.first()->rightHandSideDimension();
    for (auto c: costs) {
      CHECK(c->rightHandSideDimension() == rightHandSideDimension);
      dimension = std::max(dimension, c->minimumProblemDimension());
      diagonalWidth = std::max(diagonalWidth, c->maximumDiagonalWidth());
    }
  }
};

BandProblem makeProblem(
    const Settings& settings,
    const ProblemSummary& summary) {
  return BandProblem(
      summary.dimension,
      summary.rightHandSideDimension,
      summary.diagonalWidth,
      settings.defaultDiagReg);
}

MDArray2d initialize(
    const Settings& settings,
    const ProblemSummary& summary,
    const Array<Cost::Ptr>& costs) {
  auto dst = makeProblem(settings, summary);
  for (auto cost: costs) {
    cost->initialize(&dst);
  }
  return dst.solveAndInvalidate();
}

MDArray2d iterate(
    const Settings& settings,
    const ProblemSummary& summary,
    const Array<Cost::Ptr>& costs,
    int i, const MDArray2d& X) {
  auto problem = makeProblem(settings, summary);
  for (auto c: costs) {
    c->apply(i, X, &problem);
  }
  return problem.solveAndInvalidate();
}

Results solve(
    const Settings& settings,
    const Array<Cost::Ptr>& costs,
    const MDArray2d& Xinit) {
  ProblemSummary summary(costs);
  CHECK(Xinit.empty() || (
      Xinit.rows() == summary.dimension
      && Xinit.cols() == summary.rightHandSideDimension));
  auto X = Xinit.empty()?
      initialize(settings, summary, costs) : Xinit;
  if (X.empty()) {
    LOG(ERROR) << "Failed to initialize BandedIrls";
    return Results{X};
  }
  for (int i = 0; i < settings.iterations; i++) {
    X = iterate(settings, summary, costs, i, X);
    if (X.empty()) {
      LOG(ERROR) << "Failed to iterate BandedIrls at " << i << " iterations";
      return Results{X};
    }
  }
  return Results{X};
}

Results constantSolve(
    const Settings& settings,
    const Array<Cost::Ptr>& costs) {
  ProblemSummary summary(costs);
  auto problem = makeProblem(settings, summary);
  for (auto c: costs) {
    c->constantApply(&problem);
  }
  return Results{problem.solveAndInvalidate()};
}

}
} /* namespace sail */
