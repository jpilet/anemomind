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

template <typename T>
class UniformSamples {
 public:
  typedef Array<T> ArrayType;
  typedef UniformSamples<T> ThisType;

  UniformSamples() {}

  UniformSamples(LineKM sampling_, ArrayType samples_) :
    _sampling(sampling_), _samples(samples_) {}

  T interpolateLinear(double x) const {
    int I[2];
    double W[2];
    _sampling.makeInterpolationWeights(x, I, W);
    return W[0]*_samples[I[0]] + W[1]*_samples[I[1]];
  }

  ArrayType interpolateLinear(Arrayd X) const {
    return X.map<T>([&](double x) {return interpolateLinear(x);});
  }

  T interpolateLinearDerivative(double x) const {
    int I[2];
    double W[2];
    _sampling.makeInterpolationWeights(x, I, W);
    return (1.0/_sampling.getK())*(_samples[I[1]] - _samples[I[0]]);
  }

  ArrayType interpolateLinearDerivative(Arrayd X) const {
    return X.map<T>([&](double x) {return interpolateLinearDerivative(x);});
  }

  const LineKM &sampling() const {
    return _sampling;
  }

  const ArrayType &samples() const {
    return _samples;
  }

  int size() const {
    return _samples.size();
  }

  bool empty() const {
    return _samples.empty();
  }

  Arrayd makeCenteredX() const {
    int sampleCount = _samples.size() - 1;
    LineKM map(0, 1, _sampling(0.5), _sampling(1.5));
    Arrayd dst(sampleCount);
    for (int i = 0; i < sampleCount; i++) {
      dst[i] = map(i);
    }
    return dst;
  }

  template <typename OtherElemType>
  bool sameSamplingAs(const UniformSamples<OtherElemType> &other) const {
    return _sampling == other.sampling() &&
        _samples.size() == other.samples().size();
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
    return _sampling(0);
  }

  double high() const {
    return _sampling(_samples.size() - 1);
  }
 private:
  // Maps sample indices to time
  LineKM _sampling;

  // The samples
  ArrayType _samples;
};

typedef UniformSamples<double> UniformSamplesd;

} /* namespace mmm */

#endif /* UNIFORMSAMPLES_H_ */
