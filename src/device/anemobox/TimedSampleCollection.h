#ifndef NAUTICAL_TIMEDSAMPLECOLLECTION_H
#define NAUTICAL_TIMEDSAMPLECOLLECTION_H

#include <algorithm>
#include <deque>
#include <server/common/Optional.h>
#include <server/common/TimeStamp.h>
#include <server/common/TimedValue.h>
#include <server/nautical/types/SampledSignal.h>
#include <iostream>

namespace sail {

template<typename T>
class TimedSampleCollection : public SampledSignal<T> {
 public:
   typedef std::deque<TimedValue<T>> TimedVector;

   TimedSampleCollection(int maxBufferLength = 0)
     : _maxBufferLength(maxBufferLength),
       _maxBufferDuration(Duration<double>::seconds(0)) { }

   TimedSampleCollection(const TimedVector& entries) :

   /*
    *  This limit is chosen so that the BatchInsert test passes. But a natural
    *  choice might also be entries.size()
    */
     _maxBufferLength(std::numeric_limits<int>::max()),
     _maxBufferDuration(Duration<double>::seconds(0)) {

     insert(entries);
   }

   void insert(const TimedVector& entries);

   // This method will insert a range of elements at
   // the front of the collection. It is your responsibility
   // to ensure that (i) no element inserted is older than any
   // element already in the collection and (ii) the elements
   // being inserted are chronologically ordered.
   void insertAtFront(
       typename TimedVector::const_iterator begin,
       typename TimedVector::const_iterator end);

   // If inserting in chronological order, use append instead of insert.
   // Crashes or undefined behavior if x.time > lastTimeStamp
   void append(const TimedValue<T>& x);
   void append(TimeStamp t, T value) { append(TimedValue<T>(t, value)); }

   const TimedVector& samples() const { return _samples; }

   const TimedValue<T>& back(int backIndex) const {
     assert(backIndex >= 0 && size_t(backIndex) < _samples.size());
     return _samples[_samples.size() - 1 - backIndex];
   }

   Optional<TimedValue<T> > nearestTimedValue(TimeStamp t) const;
   Optional<T> nearest(TimeStamp t) const;

   // If 0: unlimited buffer.
   // Otherwise: after insert, only the most recent <_maxBufferLength> samples
   // are kept.
   void setMaxBufferLength(int maxBufferLength) {
     _maxBufferLength = maxBufferLength;
     trim();
   }

   // If <= 0: no time-based limit.
   // Otherwise: only samples within <_maxBufferDuration> of the most recent
   // sample are kept. This bounds the buffer by wall-clock time rather than
   // by sample count, regardless of the channel's update rate.
   void setMaxBufferDuration(Duration<double> maxBufferDuration) {
     _maxBufferDuration = maxBufferDuration;
     trim();
   }

   size_t size() const override { return _samples.size(); }

   TimedValue<T> operator[](int i) const override {
     return _samples[i];
   }

   bool empty() const { return _samples.empty(); }
   T lastValue() const { return _samples.back().value; }
   TimeStamp lastTimeStamp() const { return _samples.back().time; }

   void clear() { _samples.clear(); }

 private:
  void trim();
  void trimByDuration();
  TimedVector _samples;

  int _maxBufferLength;
  Duration<double> _maxBufferDuration;
};

template <typename T>
void TimedSampleCollection<T>::append(const TimedValue<T>& x) {
  if (_samples.size() > 0 && _samples.back().time > x.time) {
    // TODO: Including <server/common/logging.h> causes
    // compilation error when this header is included together
    // with Ceres.
    std::cerr << "WARNING: "
      << "appending sample "
      << (_samples.back().time - x.time).milliseconds()
      << " ms in the future";
  }
  if (_maxBufferLength > 0 && _samples.size() >= size_t(_maxBufferLength)) {
    _samples.pop_front();
  }
  _samples.push_back(x);
  trimByDuration();
}

template <typename T>
void TimedSampleCollection<T>::insert(const TimedVector& entries) {
  _samples.insert(_samples.end(), entries.begin(), entries.end());
  sort(_samples.begin(), _samples.end());
  trim();
}

template <typename T>
void TimedSampleCollection<T>::insertAtFront(
    typename TimedVector::const_iterator begin,
    typename TimedVector::const_iterator end) {
  assert(std::is_sorted(begin, end));
  assert(implies(
    0 < _samples.size() && begin < end,
    *(end - 1) < *_samples.begin()));
  _samples.insert(_samples.begin(), begin, end);
}


template <typename T, typename Iterator>
Optional<TimedValue<T> > findNearestTimedValue(Iterator begin, Iterator end, TimeStamp t) {
  if (begin == end) {
    return Optional<TimedValue<T> >();
  }
  const TimedValue<T> time(t, T());
  if (time < *(begin) || t > (*(end - 1)).time) {
    return Optional<TimedValue<T> >();
  }

  auto it = std::lower_bound(begin, end, time);
  if (it != begin) {
    auto prev = it;
    --prev;
    if ((prev->time - t).fabs() < (it->time - t).fabs()) {
      it = prev;
    }
  }
  return Optional<TimedValue<T> >(*it);
}

template <typename T>
Optional<TimedValue<T> > TimedSampleCollection<T>::nearestTimedValue(TimeStamp t) const {
  typedef typename TimedVector::const_iterator Iterator;
  return findNearestTimedValue<T, Iterator>(_samples.begin(), _samples.end(), t);
}


template <typename T>
Optional<T> TimedSampleCollection<T>::nearest(TimeStamp t) const {
  auto tv = nearestTimedValue(t);
  if (tv.defined()) {
    return Optional<T>(tv.get().value);
  }
  return Optional<T>();
}

template <typename T>
void TimedSampleCollection<T>::trim() {
  if (_maxBufferLength > 0) {
    int toRemove = _samples.size() - _maxBufferLength;
    if (toRemove > 0) {
      _samples.erase(_samples.begin(), _samples.begin() + toRemove);
    }
  }
  trimByDuration();
}

template <typename T>
void TimedSampleCollection<T>::trimByDuration() {
  if (_maxBufferDuration <= Duration<double>::seconds(0) || _samples.empty()) {
    return;
  }
  TimeStamp oldestToKeep = _samples.back().time - _maxBufferDuration;
  size_t toRemove = 0;
  while (toRemove < _samples.size() && _samples[toRemove].time < oldestToKeep) {
    ++toRemove;
  }
  if (toRemove > 0) {
    _samples.erase(_samples.begin(), _samples.begin() + toRemove);
  }
}

}  // namespace sail

#endif  // NAUTICAL_TIMEDSAMPLECOLLECTION_H

