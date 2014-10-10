/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "UniformSamples.h"
#include <cassert>

namespace sail {

UniformSamples::UniformSamples(LineStrip sampling_, Arrayd samples_) :
    _sampling(sampling_), _samples(samples_) {}

double UniformSamples::interpolateLinear(double x) const {
  int I[2];
  double W[2];
  _sampling.makeVertexLinearCombination(&x, I, W);
  return W[0]*_samples[I[0]] + W[1]*_samples[I[1]];
}

double UniformSamples::interpolateLinearDerivative(double x) const {
  int I[2];
  double W[2];
  _sampling.makeVertexLinearCombination(&x, I, W);
  double spacing = _sampling.getEq(0).getK();

    assert(I[0] + 1 == I[1]);
    assert(std::abs(_sampling.getEq(0)(I[1]) - _sampling.getEq(0)(I[0])
        - _sampling.getEq(0).getK()) < 1.0e-5);

  return (W[1]*_samples[I[1]] - W[0]*_samples[I[0]])/spacing;
}


}
