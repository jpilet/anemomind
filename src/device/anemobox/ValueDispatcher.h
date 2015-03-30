#ifndef VALUE_DISPATCHER_H
#define VALUE_DISPATCHER_H

#include <deque>
#include <set>

#include <server/common/TimeStamp.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

template <typename T> class ValueDispatcher;

template <typename T>
class Listener {
 public:
  Listener(Duration<> minInterval = Duration<>::seconds(0)) : minInterval_(minInterval) { }
  Duration<> minInterval() const { return minInterval; }
  virtual void onNewValue(const ValueDispatcher<T> &dispatcher) = 0;

  void notify(const ValueDispatcher<T> &dispatcher);

 private:
  TimeStamp lastNotified_;
  Duration<> minInterval_;
};

template <typename T>
struct TimedValue {
  TimedValue(T value) : time(TimeStamp::now()), value(value) { }

  TimeStamp time;
  T value;
};

template <typename T>
class ValueDispatcher {
 public:
  ValueDispatcher(int bufferLength) : bufferLength_(bufferLength) { }

  void subscribe(Listener<T> *listener) { listeners_.insert(listener); }
  int unsubscribe(Listener<T> *listener) { return listeners_.erase(listener); }

  void setValue(T value);

  T lastValue() const { return values_.front().value; }
  virtual TimeStamp lastTimeStamp() const { return values_.front().time; }

  const std::deque<TimedValue<T>>& values() const { return values_; }

 private:
  std::set<Listener<T> *> listeners_;
  std::deque<TimedValue<T>> values_;
  int bufferLength_;
};

template <typename T>
void Listener<T>::notify(const ValueDispatcher<T> &dispatcher)
{
  if (!lastNotified_.defined()
      || (dispatcher.lastTimeStamp() - lastNotified_) >= minInterval_) {
    lastNotified_ = dispatcher.lastTimeStamp();
    onNewValue(dispatcher);
  }
}

template <typename T>
void ValueDispatcher<T>::setValue(T value) {
  values_.push_front(TimedValue<T>(value));
  while (values_.size() > bufferLength_) {
    values_.pop_back();
  }
  for (Listener<T> *listener : listeners_) {
    listener->notify(*this);
  }
}

// Pre-define a few types
typedef ValueDispatcher<Angle<double>> AngleDispatcher;
typedef ValueDispatcher<Velocity<double>> VelocityDispatcher;

}  // namespace sail

#endif // VALUE_DISPATCHER_H
