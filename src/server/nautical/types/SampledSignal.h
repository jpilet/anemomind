/*
 * DiscreteTimeSignal.h
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_TYPES_DISCRETETIMESIGNAL_H_
#define SERVER_NAUTICAL_TYPES_DISCRETETIMESIGNAL_H_

#include <server/common/AbstractArray.h>
#include <server/common/TimedValue.h>

namespace sail {

// Represents a read-only sampled signal
template <typename T>
class SampledSignal : public AbstractArray<TimedValue<T> > {
public:
  typedef TimedValue<T> TimedType;
  virtual ~SampledSignal() {}

  bool chronologicallyOrdered() const {
    auto n = this->size() - 1;
    for (int i = 0; i < n; i++) {
      if ((*this)[i].time > (*this)[i + 1].time) {
        return false;
      }
    }
    return true;
  }
};

template <typename T>
class UniformlySampledSignal : public SampledSignal<T> {
public:
  virtual Duration<double> samplingPeriod() const = 0;
  virtual ~UniformlySampledSignal() {}
};

}

#endif /* SERVER_NAUTICAL_TYPES_DISCRETETIMESIGNAL_H_ */
