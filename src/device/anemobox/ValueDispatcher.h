#ifndef VALUE_DISPATCHER_H
#define VALUE_DISPATCHER_H

#include <deque>
#include <set>
#include <vector>

#include <server/common/TimeStamp.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/nautical/GeographicPosition.h>

namespace sail {

template <typename T> class ValueDispatcher;

template <typename T>
class Listener {
 public:
  Listener(Duration<> minInterval = Duration<>::seconds(0))
    : minInterval_(minInterval), listeningTo_(0) { }
  virtual ~Listener();

  Duration<> minInterval() const { return minInterval; }
  virtual void onNewValue(const ValueDispatcher<T> &dispatcher) = 0;

  void notify(const ValueDispatcher<T> &dispatcher);

  bool isListening() const { return listeningTo_ != 0; }
  void listen(ValueDispatcher<T> *dispatcher);
  void stopListening();
  ValueDispatcher<T> *listeningTo() const { return listeningTo_; };

 private:
  TimeStamp lastNotified_;
  Duration<> minInterval_;
  ValueDispatcher<T> *listeningTo_;
};

template <typename T>
struct TimedValue {
  TimedValue(TimeStamp time, T value) : time(time), value(value) { }

  TimeStamp time;
  T value;
};

template <typename T>
class ValueDispatcher {
 public:
  ValueDispatcher(Clock* clock, int bufferLength)
    : bufferLength_(bufferLength), clock(clock) { }

  void subscribe(Listener<T> *listener) { 
    listeners_.insert(listener);
    listener->listen(this);
  }
  int unsubscribe(Listener<T> *listener) { return listeners_.erase(listener); }

  void setValue(T value);

  bool hasValue() const { return values_.size() > 0; }
  T lastValue() const { return values_.front().value; }
  virtual TimeStamp lastTimeStamp() const { return values_.front().time; }

  const std::deque<TimedValue<T>>& values() const { return values_; }

 private:
  std::set<Listener<T> *> listeners_;
  std::deque<TimedValue<T>> values_;
  size_t bufferLength_;
  Clock* clock;
};

template <typename T>
Listener<T>::~Listener() {
  stopListening();
}

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
void Listener<T>::listen(ValueDispatcher<T> *dispatcher) {
  if (listeningTo_ != dispatcher) {
    stopListening();
    listeningTo_ = dispatcher;
    listeningTo_->subscribe(this);
  }
}

template <typename T>
void Listener<T>::stopListening() {
  if (isListening()) {
    listeningTo_->unsubscribe(this);
  }
  listeningTo_ = 0;
}

template <typename T>
void ValueDispatcher<T>::setValue(T value) {
  values_.push_front(TimedValue<T>(clock->currentTime(), value));
  while (values_.size() > bufferLength_) {
    values_.pop_back();
  }
  // listeners might unsubscribe during notification.
  // in that case, the listerners_ list will be modified.
  // thus, we can not iterate safely over listeners_,
  // we have to copy the list first.
  std::vector<Listener<T> *> listenersToCall;
  listenersToCall.reserve(listeners_.size());
  std::copy(listeners_.begin(),
            listeners_.end(),
            std::back_inserter(listenersToCall));
  for (Listener<T> *listener : listenersToCall) {
    listener->notify(*this);
  }
}

// Pre-define a few types
typedef ValueDispatcher<Angle<double>> AngleDispatcher;
typedef ValueDispatcher<Velocity<double>> VelocityDispatcher;
typedef ValueDispatcher<Length<double>> LengthDispatcher;
typedef ValueDispatcher<GeographicPosition<double>> GeoPosDispatcher;
typedef ValueDispatcher<TimeStamp> TimeStampDispatcher;

}  // namespace sail

#endif // VALUE_DISPATCHER_H
