/*
 * FrequencyLimiter.h
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_FREQUENCYLIMITER_H_
#define SERVER_COMMON_FREQUENCYLIMITER_H_

#include <server/common/TimeStamp.h>

namespace sail {

class FrequencyLimiter {
public:
  FrequencyLimiter(Duration<double> minPeriod)
    : _minPeriod(minPeriod) {}

  bool accept(TimeStamp t);
private:
  Duration<double> _minPeriod;
  TimeStamp _last;
};

template <typename T>
std::function<void(T)> progressNotifier(
    const std::function<void(int, T)>& v,
    Duration<double> period = 1.0_s) {
  auto l = std::make_shared<FrequencyLimiter>(period);
  auto c = std::make_shared<int>(0);
  return [l, c, v](const T& data) {
    if (l->accept(TimeStamp::now())) {
      v(*c, data);
    }
    (*c)++;
  };
}

} /* namespace sail */

#endif /* SERVER_COMMON_FREQUENCYLIMITER_H_ */
