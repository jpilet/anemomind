/*
 *  Created on: 2014-10-09
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *
 *  A uniformly sampled signal.
 */

#ifndef UNIFORMSAMPLES_H_
#define UNIFORMSAMPLES_H_

namespace sail {

class UniformSamples {
 public:
  UniformSamples() {}

  UniformSamples(LineStrip sampling_, Arrayd samples_);

  double interpolateLinear(double x) const;
  double interpolateLinearDerivative(double x) const;

  const LineStrip &sampling() const {
    return _sampling;
  }

  const Arrayd &samples() const {
    return _samples;
  }
 private:
  // Maps sample indices to time
  LineStrip _sampling;

  // The samples
  Arrayd _samples;
};

} /* namespace mmm */

#endif /* UNIFORMSAMPLES_H_ */
