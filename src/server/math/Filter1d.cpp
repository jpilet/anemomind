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
    BandMat<T> AtA(strip.getVertexCount(), strip.getVertexCount(), width, width);
    AtA.addRegs(regOrders, regWeights);
    MDArray<T, 2> AtB(strip.getVertexCount(), 1);
    AtB.setAll(0);
    int count = X.size();
    assert(count == Y.size());
    for (int i = 0; i < count; i++) {
      int I[2];
      double W[2];
      strip.makeVertexLinearCombination(X.ptr(i), I, W);
      AtA.addNormalEq(2, I, W);
      double y = Y[i];
      AtB[I[0]] += y*W[0];
      AtB[I[1]] += y*W[1];
    }
    if (bandMatGaussElimDestructive(&AtA, &AtB)) {
      return AtB.getStorage();
    }
    return Array<T>();
  }

  class CVObjf : public AutoDiffFunction {
   public:
    CVObjf(LineStrip strip, Arrayd X, Arrayd Y,
        Array<Arrayb> crossValidationSplits);

    // OVERRIDE THESE METHODS:
    virtual void evalAD(adouble *Xin, adouble *Fout) = 0;
    virtual int inDims() {return _orders.size();}
    //  virtual int outDims() = 0;
   private:
    Arrayi _orders;
    LineStrip _strip;
    Array<Arrayd> _Xtrain, _Xvalid, _Ytrain, _Yvalid;
  };
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
