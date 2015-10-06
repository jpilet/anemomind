/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/DataFit.h>
#include <server/common/math.h>

namespace sail {
namespace DataFit {


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
