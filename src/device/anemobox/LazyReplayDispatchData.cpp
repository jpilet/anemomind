/*
 * LazyReplayDispatchData.cpp
 *
 *  Created on: 1 Jun 2017
 *      Author: jonas
 */

#include <device/anemobox/LazyReplayDispatchData.h>

namespace sail {

template <typename T>
DispatchData* makeTypedLazyReplayDispatchData(
    Clock* clock, int bufferLength,
    const std::shared_ptr<DispatchData>& prototype) {
  return new LazyReplayDispatchData<T>(
      clock, bufferLength,
      std::static_pointer_cast<TypedDispatchData<T>>(prototype));
}

DispatchData* makeLazyReplayDispatchData(
    Clock* clock, int bufferLength,
    const std::shared_ptr<DispatchData>& prototype) {
  if (!bool(prototype)) {
    return nullptr;
  }
  switch (prototype->dataCode()) {
#define CASE_MAKE_LAZY_REPLAY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
      case HANDLE: return makeTypedLazyReplayDispatchData<TYPE>(clock, bufferLength, prototype);
FOREACH_CHANNEL(CASE_MAKE_LAZY_REPLAY)
#undef CASE_MAKE_LAZY_REPLAY
  };
  return nullptr;
}

template <typename T>
bool finalizeTypedLazyReplayDispatchData(
    DispatchData* d) {
  return static_cast<LazyReplayDispatchData<T>*>(d)->finalize();
}

bool finalizeLazyReplayDispatchData(
    DispatchData* d) {
  if (!bool(d)) {
    return false;
  }
  switch (d->dataCode()) {
  #define CASE_MAKE_LAZY_REPLAY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
        case HANDLE: return finalizeTypedLazyReplayDispatchData<TYPE>(d);
  FOREACH_CHANNEL(CASE_MAKE_LAZY_REPLAY)
  #undef CASE_MAKE_LAZY_REPLAY
  };
  return false;
}

} /* namespace sail */
