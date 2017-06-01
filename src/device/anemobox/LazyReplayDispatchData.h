/*
 * LazyReplayDispatchData.h
 *
 *  Created on: 1 Jun 2017
 *      Author: jonas
 */

#ifndef DEVICE_ANEMOBOX_LAZYREPLAYDISPATCHDATA_H_
#define DEVICE_ANEMOBOX_LAZYREPLAYDISPATCHDATA_H_

#include <device/anemobox/Dispatcher.h>
#include <server/common/logging.h>

namespace sail {

template <typename T>
const typename TimedSampleCollection<T>::TimedVector& samplesOf(
    const std::shared_ptr<TypedDispatchData<T>>& src) {
  return src->dispatcher()->values().samples();
}

// Used internally by ReplayDispatcher.
template <typename T>
class LazyReplayDispatchData : public TypedDispatchData<T> {
public:
  LazyReplayDispatchData(Clock* clock, int bufferLength,
                        const std::shared_ptr<TypedDispatchData<T>>&
                          prototype)
     : TypedDispatchData<T>(prototype->dataCode(), prototype->source()),
     _dispatcher(clock, bufferLength), _prototype(prototype) {
    if (prototype) {
      _sameUpTo = samplesOf(prototype).begin();
    }
  }

  ValueDispatcher<T> *dispatcher() {
    return _finalized && _prototype?
        _prototype->dispatcher() : &_dispatcher;
  }

  const ValueDispatcher<T> *dispatcher() const {
    return _finalized && _prototype?
        _prototype->dispatcher() : &_dispatcher; }

  void setValue(T value) {
    _counter++;
    _dispatcher.setValue(value);
    stepIterator();
  }

  /*
   * Call this method once we know that no new samples will be inserted.
   * This will ensure that the 'dispatcher()' method returns a
   * ValueDispatcher<T> containing *all* data ever inserted,
   * and nothing else.
   *
   * After calling 'finalize()' it is still possible to call
   * setValue, but why would you do that? Also, there is no
   * guarantee that anyone subscribed to this object
   * will get called after you call finalize.
   *
   * It return true iff the dispatcher contains data.
   *
   */
  bool finalize() {
    if (_prototype) {
      if (_sameUpTo != samplesOf(_prototype).end()) {
        // We only have a subset, so let's migrate.
        migrateFromPrototypeToDispatcher();
      }
    }
    _finalized = true;
    return 0 < _counter;
  }

  virtual ~LazyReplayDispatchData() {}
 private:
  int _counter = 0;
  bool _finalized = false;
  ValueDispatcher<T> _dispatcher;
  typename TimedSampleCollection<T>::TimedVector::const_iterator _sameUpTo;
  std::shared_ptr<TypedDispatchData<T>> _prototype;

  void stepIterator() {
    if (_prototype) {
      if (_sameUpTo == samplesOf(_prototype).end()) {
        migrateFromPrototypeToDispatcher();
      } else {
        auto lastTime = _dispatcher.lastTimeStamp();
        auto lastValue = _dispatcher.lastValue();
        const auto& x = *_sameUpTo;
        if (lastTime == x.time && lastValue == x.value) {
          _sameUpTo++;
        } else {
          migrateFromPrototypeToDispatcher();
        }
      }
    }
  }

  void migrateFromPrototypeToDispatcher() {
    auto coll = _dispatcher.mutableValues();
    coll->setMaxBufferLength(
                std::numeric_limits<int>::max());
    int missingCount = _counter - coll->size();
    if (0 < missingCount) {
      auto b = samplesOf(_prototype).begin();
      coll->insertAtFront(b, b + missingCount);
    }
    _prototype = std::shared_ptr<TypedDispatchData<T>>();
  }
};

DispatchData* makeLazyReplayDispatchData(
    Clock* clock, int bufferLength,
    const std::shared_ptr<DispatchData>& prototype);

bool finalizeLazyReplayDispatchData(DispatchData* d);

} /* namespace sail */

#endif /* DEVICE_ANEMOBOX_LAZYREPLAYDISPATCHDATA_H_ */
