/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_NONLINEAR_ROBUSTSIGNAL_H_
#define SERVER_MATH_NONLINEAR_ROBUSTSIGNAL_H_

#include <server/common/math.h>
#include <server/common/Sampling.h>
#include <server/math/Majorize.h>

namespace sail {
namespace RobustSignal {

class RobustCost {
 public:
  RobustCost(double sigma, double scale = 1.0) :
    _sigma2(sigma*sigma), _scale(scale) {}

  double eval(double x) const {
    return _scale*evalSub(_scale*x);
  }

  double evalDerivative(double x) const {
    return sqr(_scale)*derivativeSub(_scale*x);
  }

  MajQuad majorize(double x) const {
    return MajQuad::majorize(x, eval(x), evalDerivative(x), 0.0);
  }
 private:
  double _sigma2, _scale;

  double evalSub(double x) const {
    double x2 = sqr(x);
    return x2*_sigma2/(x2 + _sigma2) - _sigma2;
  }

  double derivativeSub(double x) const {
    double f = _sigma2/(_sigma2 + sqr(x));
    return 2*x*sqr(f);
  }
};

struct Settings {
 Settings() : sigma(1.0), iters(30), lambda(0.5),
     minScale(-1), maxScale(8) {}
 double sigma;
 int iters;
 double lambda;
 int minScale, maxScale;
};

template <int N>
double calcResidual(const Observation<2> &obs, const MDArray2d &X) {
  double squaredSum = 0.0;
  const auto &w = obs.weights;
  for (int i = 0; i < N; i++) {
    double x = w.lowerWeight*X(w.lowerIndex, i) + w.upperWeight*X(w.upperIndex(), i);
    squaredSum += sqr(x - obs.data[i]);
  }
}

template <int N>
MDArray2d optimize(Sampling sampling,
    Array<Observation<N> > observations, Settings settings) {
  MDArray2d X(sampling.count(), N);
  X.setAll(0.0);
  int scale = settings.maxScale;
  for (int i = 0; i < settings.iters; i++) {
    X = iterate(scale, X, observations, settings);
    scale = adjustScale(scale, X, observations, settings);
  }
  return X;
}



}
}





#endif /* SERVER_MATH_NONLINEAR_ROBUSTSIGNAL_H_ */
