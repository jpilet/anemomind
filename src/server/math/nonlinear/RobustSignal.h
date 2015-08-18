/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_NONLINEAR_ROBUSTSIGNAL_H_
#define SERVER_MATH_NONLINEAR_ROBUSTSIGNAL_H_

namespace sail {
namespace RobustSignal {

struct Settings {
 Settings() : sigma(1.0), iters(30), lambda(0.5),
     minScale(-1), maxScale(8) {}
 double sigma;
 int iters;
 double lambda;
 int minScale, maxScale;
};

template <int N>
struct Observation {
  Sampling::Weights weights;
  double data[N];
};

template <int N>
MDArray2d optimize(Sampling sampling,
    Array<Observation<N> > observations, Settings settings) {
  int scale = settings.maxScale;
  for (int i = 0; i < settings.iters; i++) {


    scale = adjustScale(settings, observations);
  }
}



}
}





#endif /* SERVER_MATH_NONLINEAR_ROBUSTSIGNAL_H_ */
