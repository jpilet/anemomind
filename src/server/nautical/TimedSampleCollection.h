#ifndef NAUTICAL_TIMEDSAMPLECOLLECTION_H
#define NAUTICAL_TIMEDSAMPLECOLLECTION_H

#include <algorithm>
#include <device/anemobox/ValueDispatcher.h>
#include <server/common/Optional.h>

namespace sail {

template<typename T>
class TimedSampleCollection {
 public:

   typedef std::vector<TimedValue<T>> TimedVector;

   void insert(const TimedValue<T>& x);
   const TimedVector& samples() const { return _samples; }

   Optional<T> nearest(TimeStamp t);
 private:
  TimedVector _samples;
};

template <typename T>
void TimedSampleCollection<T>::insert(const TimedValue<T>& x) {
  _samples.insert(
      std::lower_bound(_samples.begin(), _samples.end(), x),
      x);
}

template <typename T>
Optional<T> TimedSampleCollection<T>::nearest(TimeStamp t) {
  const TimedValue<T> time(t, T());
  // Refuse to extrapolate.
  if (_samples.size() == 0
      || time < _samples.front()
      || t > _samples.back().time) {
    return Optional<T>();
  }

  auto it = std::lower_bound(_samples.begin(), _samples.end(), time);
  if (it != _samples.begin()) {
    auto prev = it;
    --prev;
    if ((prev->time - t).fabs() < (it->time - t).fabs()) {
      it = prev;
    }
  }
  return Optional<T>(it->value);
}


}  // namespace sail

#endif  // NAUTICAL_TIMEDSAMPLECOLLECTION_H

