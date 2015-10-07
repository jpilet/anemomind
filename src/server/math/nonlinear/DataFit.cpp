/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/DataFit.h>
#include <server/common/math.h>

namespace sail {
namespace DataFit {

void makeEye(double weight, Spani rowSpan, Spani colSpan, std::vector<Triplet> *dst) {
  int n = std::min(rowSpan.width(), colSpan.width());
  for (int i = 0; i < n; i++) {
    dst->push_back(Triplet(rowSpan.minv() + i, colSpan.minv() + i, weight));
  }
}

void makeReg(double weight, int order, CoordIndexer rowIndexer, CoordIndexer colIndexer,
    std::vector<Triplet> *dst) {
  assert(rowIndexer.dim() == colIndexer.dim());
  assert(rowIndexer.count() + order == colIndexer.count());
  Arrayd coefs = makeRegCoefs(order);
  for (auto i: rowIndexer.coordinateSpan()) {
    Spani rowSpan = rowIndexer.span(i);
    std::cout << EXPR_AND_VAL_AS_STRING(rowSpan.maxv()) << std::endl;
    for (int j = 0; j < coefs.size(); j++) {
      Spani colSpan = colIndexer.span(i + j);
      makeEye(weight*coefs[j], rowSpan, colSpan, dst);
      std::cout << EXPR_AND_VAL_AS_STRING(colSpan.maxv()) << std::endl;
    }
  }
}


Array<Spani> makeReg(int order, int firstRowOffset, int firstColOffset,
    int dim, int count, std::vector<Triplet> *dst) {
  Array<Spani> spans(count);
  Arrayd coefs = makeRegCoefs(order);
  for (int i = 0; i < count; i++) {
    int localOffset = dim*i;
    int rowOffset = firstRowOffset + localOffset;
    int colOffset = firstColOffset + localOffset;
    int nextRowOffset = rowOffset + dim;
    spans[i] = Spani(rowOffset, nextRowOffset);
    for (int ci = 0; ci < coefs.size(); ci++) {
      int coefColOffset = colOffset + dim*ci;
      double c = coefs[ci];
      for (int k = 0; k < dim; k++) {
        dst->push_back(Triplet(rowOffset + k, coefColOffset + k, c));
      }
    }
  }
  return spans;
}

double calcSquaredResidual(const Arrayd &X) {
  double sum2 = 0.0;
  for (auto x: X) {
    sum2 += x*x;
  }
  return sum2;
}

Arrayb getInliers(int dim, int inlierCount, Arrayd data) {
  int count = data.size()/dim;
  CHECK(count*dim == data.size());
  Arrayd residuals(count);
  for (int i = 0; i < count; i++) {
    residuals[i] = calcSquaredResidual(data.sliceBlock(i, dim));
  }
  Arrayd sortedResiduals = residuals.dup();
  std::sort(sortedResiduals.begin(), sortedResiduals.end());
  double thresh = sortedResiduals[inlierCount-1];
  return residuals.map<bool>([&](double x) {
    return x <= thresh;
  });
}

MDArray2d getSamples(int dim, Arrayd data) {
  int count = data.size()/dim;
  CHECK(count*dim == data.size());
  MDArray2d dst(count, dim);
  int counter = 0;
  for (int i = 0; i < count; i++) {
    for (int j = 0; j < dim; j++) {
      dst(i, j) = data[counter];
      counter++;
    }
  }
  CHECK(counter == data.size());
  return dst;
}

Results assembleResults(irls::Results solution, CoordIndexer data,
    CoordIndexer samples, double inlierThreshold) {
  auto t2 = sqr(inlierThreshold);
  MDArray2d out(samples.count(), samples.dim());
  for (int i = 0; i < samples.count(); i++) {
    for (int j = 0; j < samples.dim(); j++) {
      auto sp = samples.span(i);
      out(i, j) = solution.X[sp[j]];
    }
  }
  Arrayb inliers(data.count());
  for (auto i: data.coordinateSpan()) {
    auto sp = data.span(i);
    double x = 0.0;
    for (auto j: sp) {
      x += sqr(solution.residuals(j));
    }
    inliers[i] = x < t2;
  }
  return Results{inliers, out};
}


Results assembleResults(int dim, int sampleCount, int inlierCount,
    const Eigen::VectorXd &solution) {
  Arrayd data(solution.size(), solution.data());
  int sampleDim = dim*sampleCount;
  return Results{getInliers(dim, inlierCount, data.sliceFrom(sampleDim)),
    getSamples(dim, data.sliceTo(sampleDim))
  };
}






}
}
