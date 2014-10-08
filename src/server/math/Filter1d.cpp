/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Filter1d.h"
#include <server/math/BandMat.h>
#include <server/math/ADFunction.h>

namespace sail {

namespace {
  int getMaxOrder(Arrayi orders) {
    int maxo = 0;
    for (int i = 0; i < orders.size(); i++) {
      maxo = std::max(maxo, orders[i]);
    }
    return maxo;
  }

  template <typename T>
  Array<T> fitSignal(LineStrip strip, Arrayd X, Arrayd Y,
      Arrayi regOrders, Array<T> regWeights) {
    int maxOrder = getMaxOrder(regOrders);
    int width = maxOrder;
    BandMat<T> mat(strip.getVertexCount(), strip.getVertexCount(), width, width);

  }
}

Arrayd filter1d(LineStrip strip, Arrayd X, Arrayd Y,
    Array<Arrayb> crossValidationSplits,
    LevmarSettings s) {

}


Arrayd filter1d(Arrayd Y, Array<Arrayb> crossValidationSplits,
    LevmarSettings s) {

  int count = Y.size();
  double marg = 0.5;
  BBox<1> bbox(Span(0, count - 1 + marg));
  double spacing[1] = 1;
  Arrayd X(count);
  for (int i = 0; i < count; i++) {
    X[i] = i;
  }
  LineStrip strip(bbox, spacing);
  assert(strip.getVertexCount() == 1);
  return filter1d(strip, X, Y, crossValidationSplits, s);
}


}


} /* namespace mmm */
