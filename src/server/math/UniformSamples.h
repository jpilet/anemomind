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
#include <server/math/nonlinear/SignalUtils.h>
#include <server/common/Functional.h>

namespace sail {

template <typename T>
class UniformSamples {
 public:
  typedef Array<T> ArrayType;
  typedef UniformSamples<T> ThisType;

  UniformSamples() {}

  UniformSamples(Sampling sampling_, ArrayType samples_) :
    _sampling(sampling_), _samples(samples_) {}

  UniformSamples(LineKM sampling_, ArrayType samples_) :
    _sampling(samples_.size(), sampling_),
    _samples(samples_) {}

  T interpolateLinear(double x) const {
    auto w = _sampling.represent(x);
    return w.eval(_samples);
  }

  T interpolateLinearBounded(double x) const {
    if (x <= low()) {
      return _samples.first();
    }
    if (high() <= x) {
      return _samples.last();
    }
    return interpolateLinear(x);
  }

  ArrayType interpolateLinear(Arrayd X) const {
    return toArray(map(X, [&](double x) {return interpolateLinear(x);}));
  }

  T interpolateLinearDerivative(double x) const {
    auto w = _sampling.represent(x);
    return (1.0/_sampling.indexToX().getK())*(_samples[w.upperIndex()] - _samples[w.lowerIndex]);
  }

  ArrayType interpolateLinearDerivative(Arrayd X) const {
    return toArray(map(X, [&](double x) {return interpolateLinearDerivative(x);}));
  }

  const LineKM &indexToX() const {
    return _sampling.indexToX();
  }

  const ArrayType &samples() const {
    return _samples;
  }

  const Sampling &sampling() const {
    return _sampling;
  }

  int size() const {
    return _samples.size();
  }

  bool empty() const {
    return _samples.empty();
  }

  Arrayd makeCenteredX() const {
    int sampleCount = _samples.size() - 1;
    LineKM map(0, 1, indexToX()(0.5), indexToX()(1.5));
    Arrayd dst(sampleCount);
    for (int i = 0; i < sampleCount; i++) {
      dst[i] = map(i);
    }
    return dst;
  }

  template <typename OtherElemType>
  bool sameSamplingAs(const UniformSamples<OtherElemType> &other) const {
    return _sampling == other.sampling();
  }

  T get(int index) const {
    return _samples[index];
  }

  ThisType operator+(T x) const {
    int count = _samples.size();
    ArrayType dst(count);
    for (int i = 0; i < count; i++) {
      dst[i] = _samples[i] + x;
    }
    return ThisType(_sampling, dst);
  }

  ThisType operator-(T x) const {
    return (*this) + (-x);
  }

  double low() const {
    return _sampling.indexToX()(0);
  }

  double high() const {
    return _sampling.indexToX()(_sampling.lastIndex());
  }
 private:
  Sampling _sampling;

  // The samples
  ArrayType _samples;
};

typedef UniformSamples<double> UniformSamplesd;

} /* namespace mmm */

#endif /* UNIFORMSAMPLES_H_ */
