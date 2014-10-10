/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "UniformSamples.h"
#include <cassert>

namespace sail {

UniformSamples::UniformSamples(LineKM sampling_, Arrayd samples_) :
    _sampling(sampling_), _samples(samples_) {}

double UniformSamples::interpolateLinear(double x) const {
  int I[2];
  double W[2];
  _sampling.makeInterpolationWeights(x, I, W);
  return W[0]*_samples[I[0]] + W[1]*_samples[I[1]];
}

double UniformSamples::interpolateLinearDerivative(double x) const {
  int I[2];
  double W[2];
  _sampling.makeInterpolationWeights(x, I, W);
  return (W[1]*_samples[I[1]] - W[0]*_samples[I[0]])/_sampling.getK();
}


}
