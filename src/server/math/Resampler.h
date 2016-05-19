/*
 * Resampler.h
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_RESAMPLER_H_
#define SERVER_MATH_RESAMPLER_H_

#include <algorithm>
#include <server/common/ArrayBuilder.h>

namespace sail {
namespace Resampler {

template <typename T>
struct Endpoint {
  typedef Endpoint<T> ThisType;

  T pos;
  bool rising;

  bool operator<(const ThisType &other) {
    return pos < other.pos;
  }
};

template <typename T>
Array<Endpoint<T> > listEndpoints(const Array<T> &samples, T period) {
  int n = samples.size();
  Array<Endpoint<T> > eps(2*n);
  for (int i = 0; i < n; i++) {
    auto x = samples[i];
    auto at = 2*i;
    // Here we are essentially dilating every sample, by replacing it
    // by an interval of width 2*period
    eps[at + 0] = Endpoint<T>{x - period, true};
    eps[at + 1] = Endpoint<T>{x + period, false};
  }
  std::sort(eps.begin(), eps.end());
  return eps;
}

template <typename T>
void addNewSamples(ArrayBuilder<T> *dst, T from, T to, T period) {
  for (auto i = from; i <= to; i += period) {
    dst->add(i);
  }
}

template <typename T>
Array<T> makeNewSamplesFromEndpoints(const Array<Endpoint<T> > & eps, T period) {
  int sum = 0;
  T start = T();
  ArrayBuilder<T> newSamples;
  for (auto ep: eps) {
    sum += (ep.rising? 1 : -1);
    if (sum == 1 && ep.rising) {
      start = ep.pos;
    } else if (sum == 0 && !ep.rising) {
      // Here we are eroding it, by cutting 'period' from each side
      addNewSamples(&newSamples, start+period, ep.pos-period, period);
    }
  }
  return newSamples.get();
}

template <typename T>
Array<T> resample(const Array<T> &irregularSamples, T newSamplingPeriod) {
  return makeNewSamplesFromEndpoints(
      listEndpoints(irregularSamples, newSamplingPeriod),
      newSamplingPeriod);
}

}
}



#endif /* SERVER_MATH_RESAMPLER_H_ */
