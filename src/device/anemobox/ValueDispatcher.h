#ifndef VALUE_DISPATCHER_H
#define VALUE_DISPATCHER_H

#include <deque>
#include <set>
#include <vector>

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <device/anemobox/BinarySignal.h>
#include <device/anemobox/TimedSampleCollection.h>
#include <server/nautical/AbsoluteOrientation.h>
#include <server/nautical/GeographicPosition.h>
#include <device/anemobox/LocalArrayCopy.h>
#include <iostream>

namespace sail {

template <typename T> class ValueDispatcher;

template <typename T>
class Listener : boost::noncopyable {
 public:
  Listener(Duration<> minInterval = Duration<>::seconds(0))
    : minInterval_(minInterval) { }
  virtual ~Listener();

  Duration<> minInterval() const { return minInterval; }
  virtual void onNewValue(const ValueDispatcher<T> &dispatcher) = 0;

  void notify(const ValueDispatcher<T> &dispatcher);

  bool isListeningTo(ValueDispatcher<T>* x) const { return 0 < listeningTo_.count(x); }
  void listenTo(ValueDispatcher<T> *dispatcher);
  void stopListeningTo(ValueDispatcher<T>* x);
  void stopListeningToAll();
  const std::set<ValueDispatcher<T>*>& listeningTo() const { return listeningTo_; };

  static void safelyNotifyListenerSet(const std::set<Listener<T> *>& listeners,
                                      const ValueDispatcher<T> &dispatcher) {
    // listeners might unsubscribe during notification.
    // in that case, the listerners list will be modified.
    // thus, we can not iterate safely over listeners,
    // we have to copy the list first.
    LocalArrayCopy<Listener<T> *, 30> listenersToCall(
        listeners.begin(), listeners.end());
    for (Listener<T> *listener : listenersToCall) {
      listener->notify(dispatcher);
    }
  }

 private:
  TimeStamp lastNotified_;
  Duration<> minInterval_;
  std::set<ValueDispatcher<T>*> listeningTo_;
};

template <typename T>
class ValueDispatcher {
 public:
  ValueDispatcher(Clock* clock, int bufferLength)
    : values_(bufferLength), clock_(clock) { }

  void subscribe(Listener<T> *listener) {
    listeners_.insert(listener);
    listener->listenTo(this);
  }
  int unsubscribe(Listener<T> *listener) {
    return listeners_.erase(listener);
  }

  virtual void setValue(T value);

  virtual bool hasValue() const { return values_.size() > 0; }
  virtual T lastValue() const { return values_.lastValue(); }
  virtual TimeStamp lastTimeStamp() const { return values_.lastTimeStamp(); }

  virtual const TimedSampleCollection<T>& values() const { return values_; }

  TimedSampleCollection<T>* mutableValues() {return &values_;}

  void insert(const typename TimedSampleCollection<T>::TimedVector& values) {
    values_.insert(values);
  }

  virtual Clock* clock() const { return clock_; }

  const std::set<Listener<T> *> &listeners() const {
    return listeners_;
  }

  virtual ~ValueDispatcher() {}
 protected:
  std::set<Listener<T> *> listeners_;
  TimedSampleCollection<T> values_;
  Clock* clock_;
};

template <typename T>
Listener<T>::~Listener() {
  stopListeningToAll();
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
void Listener<T>::listenTo(ValueDispatcher<T> *dispatcher) {
  if (!isListeningTo(dispatcher)) {
    listeningTo_.insert(dispatcher);
    dispatcher->subscribe(this);
  }
}

template <typename T>
void Listener<T>::stopListeningTo(ValueDispatcher<T>* x) {
  if (isListeningTo(x)) {
    x->unsubscribe(this);
    listeningTo_.erase(x);
  }
}

template <typename T>
void Listener<T>::stopListeningToAll() {
  for (auto x: listeningTo_) {
    x->unsubscribe(this);
  }
  listeningTo_.clear();
}

template <typename T>
void ValueDispatcher<T>::setValue(T value) {
  auto currentTime = clock_->currentTime();
  if (currentTime.defined()) {
    values_.append(TimedValue<T>(currentTime, value));
    Listener<T>::safelyNotifyListenerSet(listeners_, *this);
  } else {
    std::cerr << "ValueDispatcher<T>::setValue: Dispatcher returned undefined time";
  }
}

// Pre-define a few types
typedef ValueDispatcher<Angle<double>> AngleDispatcher;
typedef ValueDispatcher<Velocity<double>> VelocityDispatcher;
typedef ValueDispatcher<Length<double>> LengthDispatcher;
typedef ValueDispatcher<GeographicPosition<double>> GeoPosDispatcher;
typedef ValueDispatcher<TimeStamp> TimeStampDispatcher;
typedef ValueDispatcher<AbsoluteOrientation> AbsoluteOrientationDispatcher;
typedef ValueDispatcher<BinaryEdge> BinaryEdgeDispatcher;

template <typename T>
class ValueDispatcherProxy : Listener<T>, public ValueDispatcher<T> {
 public:
  ValueDispatcherProxy() : ValueDispatcher<T>(0, 1), _forward(0) { }

  virtual void setValue(T value) { if (_forward) _forward->setValue(value); }

  virtual bool hasValue() const { return _forward && _forward->hasValue(); }
  virtual T lastValue() const { return _forward ? _forward->lastValue() : T(); }
  virtual TimeStamp lastTimeStamp() const { return _forward->lastTimeStamp(); }

  virtual const TimedSampleCollection<T>& values() const {
    return _forward ? _forward->values() : emptyValues_;
  }

  virtual Clock* clock() const { return _forward ? _forward->clock() : 0; }

  void proxy(ValueDispatcher<T> *dispatcher) {
    this->stopListeningTo(_forward); // Is this OK?
    _forward = dispatcher;
    if (_forward) {
      _forward->subscribe(this);
    }
  }

  bool hasDispatcher() const { return _forward != nullptr; }

  virtual ~ValueDispatcherProxy() {}
 protected:
  virtual void onNewValue(const ValueDispatcher<T> &dispatcher) {
    Listener<T>::safelyNotifyListenerSet(this->listeners_, *this);
  }
 private:
  TimedSampleCollection<T> emptyValues_;
  ValueDispatcher<T> *_forward;
};

}  // namespace sail

#endif // VALUE_DISPATCHER_H
