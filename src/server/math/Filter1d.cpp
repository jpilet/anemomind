/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Filter1d.h"
#include <server/math/BandMat.h>
#include <server/math/ADFunction.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/nonlinear/GridFitter.h>
#include <server/math/armaadolc.h>

namespace sail {

namespace {
  void fillEmptySplits(Array<Arrayb> splits, int len) {
    for (int i = 0; i < splits.size(); i++) {
      if (splits[i].empty()) {
        splits[i] = makeRandomSplit(len);
      }
      assert(splits[i].size() == len);
    }
  }

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
    assert(X.size() == Y.size());
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
        Array<Arrayb> crossValidationSplits, Arrayi regOrders);

    void evalAD(adouble *Xin, adouble *Fout);
    int inDims() {return _regOrders.size();}
    int outDims() {return _outDims;}
   private:
    Arrayi _regOrders;
    LineStrip _strip;
    Array<Arrayd> _Xtrain, _Xvalid, _Ytrain, _Yvalid;
    int _outDims, _splitCount;
  };

  CVObjf::CVObjf(LineStrip strip, Arrayd X, Arrayd Y,
      Array<Arrayb> crossValidationSplits, Arrayi regOrders) :
          _strip(strip),
          _regOrders(regOrders),
          _splitCount(crossValidationSplits.size()) {
    _Xtrain.create(_splitCount);
    _Xvalid.create(_splitCount);
    _Ytrain.create(_splitCount);
    _Yvalid.create(_splitCount);
    _outDims = 0;
    for (int i = 0; i < _splitCount; i++) {
      Arrayb t = crossValidationSplits[i];
      Arrayb v = neg(t);
      _Xtrain[i] = X.slice(t);
      _Xvalid[i] = X.slice(v);
      _Ytrain[i] = Y.slice(t);
      _Yvalid[i] = Y.slice(v);
      _outDims += _Yvalid.size();
    }
  }

  void computeErrors(LineStrip strip, Arrayad vertices,
      Arrayd X, Arrayd Y, Arrayad outErrors) {
    int count = X.size();
    assert(count == Y.size());
    assert(count == outErrors.size());
    for (int i = 0; i < count; i++) {
      int I[2];
      double W[2];
      strip.makeVertexLinearCombination(X.ptr(i), I, W);
      outErrors[i] = W[0]*vertices[I[0]] + W[1]*vertices[I[1]] - Y[i];
    }
  }

  void CVObjf::evalAD(adouble *Xin, adouble *Fout) {
    Arrayad regWeights(inDims(), Xin);
    Arrayad errors(outDims(), Fout);
    int offset = 0;
    for (int i = 0; i < _splitCount; i++) {
      int next = offset + _Yvalid.size();
      Arrayad vertices = fitSignal(_strip, _Xtrain[i], _Ytrain[i],
          _regOrders, regWeights);
      if (vertices.empty()) {
        vertices = Arrayad::fill(_strip.getVertexCount(), adouble(1.0e9));
      }
      computeErrors(_strip, vertices, _Xvalid[i], _Yvalid[i],
          errors.slice(offset, next));
    }
    assert(offset == outDims());
  }
}

Arrayd filter1d(LineStrip strip, Arrayd X, Arrayd Y,
    Arrayi regOrders, Arrayd initWeights,
    Array<Arrayb> crossValidationSplits,
    LevmarSettings settings) {
  assert(regOrders.size() == initWeights.size());

  // Tune the regularization weights using cross validation
  CVObjf objf(strip, X, Y, crossValidationSplits, regOrders);
  LevmarState state(initWeights);
  state.minimize(settings, objf);

  // Using the tuned regularization weights, return the fitted signal.
  return fitSignal(strip, X, Y, regOrders, state.getXArray(false));
}


Arrayd filter1d(Arrayd Y, Arrayi orders, Arrayd weights, Array<Arrayb> crossValidationSplits,
    LevmarSettings s) {

  int count = Y.size();
  double marg = 0.5;
  BBox1d bbox(Spand(0, count - 1 + marg));
  double spacing[1] = {1};
  Arrayd X(count);
  for (int i = 0; i < count; i++) {
    X[i] = i;
  }
  LineStrip strip(bbox, spacing);
  assert(strip.getVertexCount() == 1);
  return filter1d(strip, X, Y, orders, weights, crossValidationSplits, s);
}

Arrayd filter1d(Arrayd X,
    int order, double initWeight,
    Array<Arrayb> crossValidationSplits,
    LevmarSettings s) {
    assert(!crossValidationSplits.empty());

    fillEmptySplits(crossValidationSplits, X.size());

    return filter1d(X, order, initWeight, crossValidationSplits, s);
}





} /* namespace mmm */
