/*
 *  Created on: 2014-03-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "SignalFit.h"
#include <server/math/ADFunction.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/armaadolc.h>
#include <server/common/ConcatFunction.h>
#include <server/common/MeanStabilizer.h>

namespace sail {

template <typename T>
Array<T> fitLineStripT(LineStrip strip,
    Arrayi orders, Array<T> regWeights,
    Arrayd X, Arrayd Y) {
  assert(orders.size() == regWeights.size());
  int count = X.size();
  int n = strip.getVertexCount();
  BandMat<T> A(n, n, 2, 2);
  A.setAll(0.0);
  A.addRegs(orders, regWeights);
  MDArray<T, 2> B(n, 1);
  B.setAll(0.0);
  for (int i = 0; i < count; i++) {
    int I[2];
    double W[2];
    strip.makeVertexLinearCombination(X.ptr(i), I, W);
    A.addNormalEq(2, I, W);
    double y = Y[i];
    B(I[0], 0) += W[0]*y;
    B(I[1], 0) += W[1]*y;
  }
  bandMatGaussElimDestructive<T>(&A, &B);
  assert(B.isContinuous());
  return B.getStorage();
}

Arrayd fitLineStrip(LineStrip strip,
    Arrayi orders, Arrayd regWeights,
    Arrayd X, Arrayd Y) {
    return fitLineStripT<double>(strip, orders, regWeights, X, Y);
}

Arrayi makeDefaultRegOrders(int n) {return makeRange(n, 1);}

template <typename T>
Array<T> fitLineStripT(LineStrip strip,
    Array<T> regWeights,
    Arrayd X, Arrayd Y) {
    return fitLineStripT<T>(strip, makeDefaultRegOrders(regWeights.size()),
        regWeights, X, Y);
}

namespace {
  class Objf : public AutoDiffFunction {
   public:
    Objf(LineStrip strip, Arrayd X, Arrayd Y, Array<Arrayb> splits, Arrayi orders);
    void evalAD(adouble *Xin, adouble *Fout);
    int inDims() {return _orders.size();}
    int outDims() {return _splits.size()*obsCount();}
   private:
    int obsCount() {return _X.size();}
    Arrayi _orders;
    LineStrip _strip;
    Arrayd _X, _Y;
    Array<Arrayb> _splits;

    void evalForSplit(adouble *Xin, adouble *Fout, Arrayb split);
    int evalForSplitSub(adouble *Xin, adouble *Fout, Arrayb split, bool flip);
  };

  Objf::Objf(LineStrip strip, Arrayd X, Arrayd Y, Array<Arrayb> splits, Arrayi orders) :
      _strip(strip),
      _X(X),
      _Y(Y),
      _splits(splits),
      _orders(orders) {
      assert(_X.size() == _Y.size());
  }

  void Objf::evalAD(adouble *Xin, adouble *Fout) {
    int oc = obsCount();
    for (int i = 0; i < _splits.size(); i++) {
      int offset = i*oc;
      evalForSplit(Xin, Fout + offset, _splits[i]);
    }
  }

  void Objf::evalForSplit(adouble *Xin, adouble *Fout, Arrayb split) {
    assert(obsCount() == split.size());
    int a = evalForSplitSub(Xin, Fout, split, false);
    int b = evalForSplitSub(Xin, Fout + a, split, true);
    assert(a + b == obsCount());
  }


  int Objf::evalForSplitSub(adouble *Xin, adouble *Fout, Arrayb split, bool flip) {
    std::function<bool(int)> train = [&] (int index) {return split[index] == flip;};
    std::function<bool(int)> test = [&] (int index) {return !train(index);};

    Arrayd Xtrain = _X.slice(train);
    Arrayd Ytrain = _Y.slice(train);
    Arrayd Xtest = _X.slice(test);
    Arrayd Ytest = _Y.slice(test);
    assert(Xtrain.size() + Xtest.size() == _X.size());

    Arrayad regs(inDims(), Xin);
    Arrayad vertices = fitLineStripT(_strip, _orders, regs, Xtrain, Ytrain);
    int counter = 0;
    int n = Xtest.size();
    for (int i = 0; i < n; i++) {
      int I[2];
      double W[2];
      _strip.makeVertexLinearCombination(Xtest.ptr(i), I, W);
      Fout[i] = W[0]*vertices[I[0]] + W[1]*vertices[I[1]] - Ytest[i];
    }

    return Xtest.size();
  }
}

Arrayd fitLineStripTuneRegParams(LineStrip strip, Arrayi orders, Arrayd initRegs, Arrayd X, Arrayd Y,
    Array<Arrayb> splits,
    const LevmarSettings &settings) {


  Objf objf(strip, X, Y, splits, orders);

  LevmarState state(initRegs);
  state.minimize(settings, objf);
  return state.getXArray().dup();
}

SignalFitResults fitLineStripAutoTune(LineStrip strip, Arrayi orders, Arrayd initRegs, Arrayd X, Arrayd Y,
    Array<Arrayb> splits,
    const LevmarSettings &settings) {
  if (initRegs.empty()) {
    initRegs = Arrayd(orders.size());
    initRegs.setTo(1.0);
  }
  Arrayd regs = fitLineStripTuneRegParams(strip, orders, initRegs, X, Y, splits, settings);
  return SignalFitResults(orders, regs, fitLineStripT(strip, regs, X, Y));
}

SignalFitResults fitLineStripAutoTune(LineStrip strip, Arrayd initRegs, Arrayd X, Arrayd Y,
    Array<Arrayb> splits,
    const LevmarSettings &settings) {
  return fitLineStripAutoTune(strip, makeDefaultRegOrders(initRegs.size()), initRegs,
      X, Y, splits, settings);
}

SignalFitResults fitLineStripAutoTune(LineStrip strip, Arrayi orders, Arrayd X, Arrayd Y,
    Array<Arrayb> splits,
    const LevmarSettings &settings) {
  return fitLineStripAutoTune(strip, orders, Arrayd(),
      X, Y, splits, settings);

}


} /* namespace sail */
