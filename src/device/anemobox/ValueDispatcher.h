#ifndef VALUE_DISPATCHER_H
#define VALUE_DISPATCHER_H

#include <deque>
#include <set>
#include <vector>

#include <server/common/TimeStamp.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/nautical/AbsoluteOrientation.h>
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

  static void safelyNotifyListenerSet(const std::set<Listener<T> *>& listeners,
                                      const ValueDispatcher<T> &dispatcher) {
    // listeners might unsubscribe during notification.
    // in that case, the listerners list will be modified.
    // thus, we can not iterate safely over listeners,
    // we have to copy the list first.
    std::vector<Listener<T> *> listenersToCall;
    listenersToCall.reserve(listeners.size());
    std::copy(listeners.begin(),
              listeners.end(),
              std::back_inserter(listenersToCall));
    for (Listener<T> *listener : listenersToCall) {
      listener->notify(dispatcher);
    }
  }

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
    : bufferLength_(bufferLength), clock_(clock) { }

  void subscribe(Listener<T> *listener) { 
    listeners_.insert(listener);
    listener->listen(this);
  }
  int unsubscribe(Listener<T> *listener) { return listeners_.erase(listener); }

  virtual void setValue(T value);

  virtual bool hasValue() const { return values_.size() > 0; }
  virtual T lastValue() const { return values_.front().value; }
  virtual TimeStamp lastTimeStamp() const { return values_.front().time; }

  virtual const std::deque<TimedValue<T>>& values() const { return values_; }

  virtual Clock* clock() const { return clock_; }
 protected:
  std::set<Listener<T> *> listeners_;
  std::deque<TimedValue<T>> values_;
  size_t bufferLength_;
  Clock* clock_;
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
  values_.push_front(TimedValue<T>(clock_->currentTime(), value));
  while (values_.size() > bufferLength_) {
    values_.pop_back();
  }
  Listener<T>::safelyNotifyListenerSet(listeners_, *this);
}

// Pre-define a few types
typedef ValueDispatcher<Angle<double>> AngleDispatcher;
typedef ValueDispatcher<Velocity<double>> VelocityDispatcher;
typedef ValueDispatcher<Length<double>> LengthDispatcher;
typedef ValueDispatcher<GeographicPosition<double>> GeoPosDispatcher;
typedef ValueDispatcher<TimeStamp> TimeStampDispatcher;
typedef ValueDispatcher<AbsoluteOrientation> AbsoluteOrientationDispatcher;

template <typename T>
class ValueDispatcherProxy : Listener<T>, public ValueDispatcher<T> {
 public:
  ValueDispatcherProxy() : ValueDispatcher<T>(0, 1), _forward(0) { }

  virtual void setValue(T value) { if (_forward) _forward->setValue(value); }

  virtual bool hasValue() const { return _forward && _forward->hasValue(); }
  virtual T lastValue() const { return _forward ? _forward->lastValue() : T(); }
  virtual TimeStamp lastTimeStamp() const { return _forward->lastTimeStamp(); }

  virtual const std::deque<TimedValue<T>>& values() const {
    return _forward ? _forward->values() : emptyValues_;
  }

  virtual Clock* clock() const { return _forward ? _forward->clock() : 0; }

  void proxy(ValueDispatcher<T> *dispatcher) {
    this->stopListening();
    _forward = dispatcher;
    if (_forward) {
      _forward->subscribe(this);
    }
  }

  bool hasDispatcher() const { return _forward != nullptr; }


 protected:
  virtual void onNewValue(const ValueDispatcher<T> &dispatcher) {
    Listener<T>::safelyNotifyListenerSet(this->listeners_, *this);
  }

 private:
  std::deque<TimedValue<T>> emptyValues_;
  ValueDispatcher<T> *_forward;
};

}  // namespace sail

#endif // VALUE_DISPATCHER_H
