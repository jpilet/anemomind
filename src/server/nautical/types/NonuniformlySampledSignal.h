/*
 * NonuniformlySampledSignal.h
 *
 *  Created on: Jun 2, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_TYPES_NONUNIFORMLYSAMPLEDSIGNAL_H_
#define SERVER_NAUTICAL_TYPES_NONUNIFORMLYSAMPLEDSIGNAL_H_

#include <server/nautical/types/SampledSignal.h>
#include <server/common/Array.h>

namespace sail {

template <typename T>
class NonuniformlySampledSignal : public SampledSignal<T> {
public:
  NonuniformlySampledSignal() {}

  NonuniformlySampledSignal(const Array<TimedValue<T> > &src) :
    _samples(src) {
    assert(std::is_sorted(src.begin(), src.end()));
  }

  TimedValue<T> operator[] (int i) const override {
    return _samples[i];
  }

  int size() const override {
    return _samples.size();
  }
private:
  Array<TimedValue<T> > _samples;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_TYPES_NONUNIFORMLYSAMPLEDSIGNAL_H_ */
