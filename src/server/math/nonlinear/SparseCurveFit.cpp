/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/SparseCurveFit.h>
#include <server/common/math.h>

namespace sail {
namespace SparseCurveFit {


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


}
}


