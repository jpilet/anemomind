/*
 *  Created on: 2014-10-09
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *
 *  A uniformly sampled signal.
 */

#ifndef UNIFORMSAMPLES_H_
#define UNIFORMSAMPLES_H_

#include <server/common/LineKM.h>
#include <server/common/Array.h>

namespace sail {

class UniformSamples {
 public:
  UniformSamples() {}

  UniformSamples(LineKM sampling_, Arrayd samples_);

  double interpolateLinear(double x) const;
  double interpolateLinearDerivative(double x) const;

  const LineKM &sampling() const {
    return _sampling;
  }

  const Arrayd &samples() const {
    return _samples;
  }
 private:
  // Maps sample indices to time
  LineKM _sampling;

  // The samples
  Arrayd _samples;
};

} /* namespace mmm */

#endif /* UNIFORMSAMPLES_H_ */
