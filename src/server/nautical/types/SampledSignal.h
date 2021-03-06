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
#include <server/common/Optional.h>
#include <algorithm>

namespace sail {

// Represents a read-only sampled signal
template <typename T>
class SampledSignal : public AbstractArray<TimedValue<T> > {
public:
  typedef TimedValue<T> TimedType;
  virtual ~SampledSignal() {}

  TimedType sample(int i) const {
    return (*this)[i];
  }

  bool chronologicallyOrdered() const {
    auto n = this->size() - 1;
    for (int i = 0; i < n; i++) {
      if (sample(i).time > sample(i + 1).time) {
        return false;
      }
    }
    return true;
  }

  Optional<TimedType> evaluate(TimeStamp t) const {
    if (this->empty()) {
      return Optional<TimedType>();
    }
    int lastIndex = this->size() - 1;
    int binIndex = std::upper_bound(this->begin(), this->end() - 1, t)
      - this->begin() - 1;
    if (binIndex < 0) {
      return sample(0);
    } else if (lastIndex <= binIndex) {
      return sample(lastIndex);
    }
    auto a = sample(binIndex);
    auto b = sample(binIndex + 1);
    return t - a.time < b.time - t? a : b;
  }
};

}

#endif /* SERVER_NAUTICAL_TYPES_DISCRETETIMESIGNAL_H_ */
