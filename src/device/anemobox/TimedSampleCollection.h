#ifndef NAUTICAL_TIMEDSAMPLECOLLECTION_H
#define NAUTICAL_TIMEDSAMPLECOLLECTION_H

#include <algorithm>
#include <deque>
#include <server/common/Optional.h>
#include <server/common/TimeStamp.h>
#include <server/common/logging.h>

namespace sail {

template <typename T>
struct TimedValue {
  TimedValue(TimeStamp time, T value) : time(time), value(value) { }

  TimeStamp time;
  T value;

  bool operator < (const TimedValue<T>& other) const {
    return time < other.time;
  }
};

template<typename T>
class TimedSampleCollection {
 public:
   typedef std::deque<TimedValue<T>> TimedVector;

   TimedSampleCollection(int maxBufferLength = 0)
     : _maxBufferLength(maxBufferLength) { }

   TimedSampleCollection(const TimedVector& entries) :

   /*
    *  This limit is chosen so that the BatchInsert test passes. But a natural
    *  choice might also be entries.size()
    */
     _maxBufferLength(std::numeric_limits<int>::max()) {

     insert(entries);
   }

   void insert(const TimedVector& entries);

   // If inserting in chronological order, use append instead of insert.
   // Crashes or undefined behavior if x.time > lastTimeStamp
   void append(const TimedValue<T>& x);

   const TimedVector& samples() const { return _samples; }

   const TimedValue<T>& back(int backIndex) const {
     assert(backIndex >= 0 && size_t(backIndex) < _samples.size());
     return _samples[_samples.size() - 1 - backIndex];
   }

   Optional<T> nearest(TimeStamp t);

   // If 0: unlimited buffer.
   // Otherwise: after insert, only the most recent <_maxBufferLength> samples
   // are kept.
   void setMaxBufferLength(int maxBufferLength) {
     _maxBufferLength = maxBufferLength;
     trim();
   }

   size_t size() const { return _samples.size(); }
   T lastValue() const { return _samples.back().value; }
   TimeStamp lastTimeStamp() const { return _samples.back().time; }
 private:
  void trim();
  TimedVector _samples;

  int _maxBufferLength;
};

template <typename T>
void TimedSampleCollection<T>::append(const TimedValue<T>& x) {
  if (_samples.size() > 0 && _samples.back().time > x.time) {
    LOG(WARNING)
      << "appending sample "
      << (_samples.back().time - x.time).milliseconds()
      << " ms in the future";
  }
  if (_maxBufferLength > 0 && _samples.size() >= size_t(_maxBufferLength)) {
    _samples.pop_front();
  }
  _samples.push_back(x);
}

template <typename T>
void TimedSampleCollection<T>::insert(const TimedVector& entries) {
  _samples.insert(_samples.end(), entries.begin(), entries.end());
  sort(_samples.begin(), _samples.end());
  trim();
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

template <typename T>
void TimedSampleCollection<T>::trim() {
  int toRemove = _samples.size() - _maxBufferLength;
  if (toRemove > 0) {
    _samples.erase(_samples.begin(), _samples.begin() + toRemove);
  }
}

}  // namespace sail

#endif  // NAUTICAL_TIMEDSAMPLECOLLECTION_H

