/*
 *  Created on: 2014-10-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/GeneralizedTV.h>
#include <server/math/BandMat.h>
#include <server/common/logging.h>

namespace sail {


GeneralizedTV::GeneralizedTV(int iters, double minv, double gaussElimTol) :
   _iters(iters), _minv(minv), _gaussElimTol(gaussElimTol) {}

UniformSamples GeneralizedTV::filter(UniformSamples initialSignal,
                Arrayd X,
                Arrayd Y,
                int order,
                double regularization) const {

  CHECK(X.size() == Y.size());

  // A regularization weight of 0 will probably mean that the problem becomes ill-posed.
  // and may fail.
  CHECK_LT(0.0, regularization);

  Arrayd coefs = BandMatInternal::makeCoefs(order);

  UniformSamples signal = initialSignal;

  for (int i = 0; i < _iters; i++) {
    UniformSamples bak = signal;
    signal = step(signal, X, Y, coefs, regularization);

    // If Gauss elimination would fail for some reason,
    // return the signal we had just before. Or should
    // we return nothing, so that the caller has a chance
    // to see that and take action?
    if (signal.empty()) {
      LOG(WARNING) << "GeneralizedTV did not converge.";
      return bak;
    }
  }
  return signal;
}

namespace {

  UniformSamples makeInitialSignal(Arrayd Y) {
    int count = Y.size();
    Arrayd samples(count+1);
    Y.copyToSafe(samples.sliceBut(1));
    samples.last() = Y.last();
    return UniformSamples(LineKM(1.0, -0.5), samples);
  }

}

Arrayd GeneralizedTV::makeDefaultX(int size) {
  return Arrayd::fill(size, [](int i) {return double(i);});
}


UniformSamples GeneralizedTV::filter(Arrayd Y, int order,
    double regularization) const {
  return filter(makeInitialSignal(Y),
      makeDefaultX(Y.size()), Y, order, regularization);
}

namespace {
  void addObservation(LineKM sampling, double x, double y,
      BandMat<double> *AtA, MDArray2d *AtB) {
    int I[2];
    double W[2];
    sampling.makeInterpolationWeights(x, I, W);
    AtA->addNormalEq(2, I, W);
    for (int i = 0; i < 2; i++) {
      (*AtB)(I[i], 0) += W[i]*y;
    }
  }

  void buildDataTerm(LineKM sampling, Arrayd X, Arrayd Y,
      BandMat<double> *AtA, MDArray2d *AtB) {
    int count = X.size();
    for (int i = 0; i < count; i++) {
      addObservation(sampling, X[i], Y[i], AtA, AtB);
    }
  }

  double evaluateReg(int offset, const Arrayd &x, const Arrayd &coefs) {
    double val = 0.0;
    for (int i = 0; i < coefs.size(); i++) {
      val += x[offset + i]*coefs[i];
    }
    return val;
  }

  // Derivative of the function f(x) = reg*|x|, evaluated at 'x'
  double calcSlope(double x, double reg) {
    int sign = (x < 0? -1 : 1);
    return sign*reg;
  }

  // Returns the a coefficient 'a', such that the derivative
  // of the majorizing function f(x) = a*x^2 equals 'slope'.
  // The derivative of f(x) is f'(x) = 2*a*x and we solve
  // 2*a*x = slope <=> a = slope/(2*x);
  double calcSquareWeight(double x, double slope) {
    return slope/(2*x);
  }

  // Remap x to be non-zero in order to avoid division by zero.
  double mapToNonZero(double x, double minv) {
    if (x < -minv || minv < x) {
      return x;
    }

    // Return the closest acceptable non-zero value.
    if (x < 0) {
      return -minv;
    }
    return minv;

  }

  void buildRegTerm(Arrayd X, double regularization, Arrayd coefs,
    BandMat<double> *AtA, MDArray2d *AtB, double minv) {

    // Build a least squares problem resulting
    // from the Majorization-Minimization technique.
    int count = X.size() - coefs.size() + 1;
    for (int i = 0; i < count; i++) {
      double x = mapToNonZero(evaluateReg(i, X, coefs), minv);
      double slope = calcSlope(x, regularization);
      double squareWeight = calcSquareWeight(x, slope);
      AtA->addRegAt(i, coefs, squareWeight);
    }
  }
}

UniformSamples GeneralizedTV::step(UniformSamples signal,
    Arrayd X, Arrayd Y, Arrayd coefs, double regularization) const {
  int order = coefs.size() - 1;
  BandMat<double> AtA(signal.size(), signal.size(), order, order);
  MDArray2d AtB(signal.size(), 1);
  AtB.setAll(0.0);
  buildDataTerm(signal.sampling(), X, Y, &AtA, &AtB);
  buildRegTerm(signal.samples(), regularization, coefs, &AtA, &AtB, _minv);

  if (bandMatGaussElimDestructive(&AtA, &AtB, _gaussElimTol)) {
    return UniformSamples(signal.sampling(), AtB.getStorage());
  }
  return UniformSamples(signal.sampling(), Arrayd());
}



}
