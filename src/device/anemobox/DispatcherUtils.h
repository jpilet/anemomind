/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef DEVICE_ANEMOBOX_DISPATCHERUTILS_H_
#define DEVICE_ANEMOBOX_DISPATCHERUTILS_H_

#include <memory>
#include <device/anemobox/Dispatcher.h>

namespace sail {

// Visit every channel of the dispatcher, apply the Mapper to it, and return a new
// dispatcher with all channels mapped.
template <typename Mapper>
std::shared_ptr<Dispatcher> mapDispatcherChannels(const std::shared_ptr<Dispatcher> &dispatcher, Mapper *m) {
  auto dst = std::make_shared<Dispatcher>();

  // Setting the priorities should be done first!
  for (const auto &codeAndSources: dispatcher->allSources()) {
    auto c = codeAndSources.first;
    for (const auto &kv: codeAndSources.second) {
      dst->setSourcePriority(kv.first, dispatcher->sourcePriority(kv.first));
    }
  }

  for (const auto &codeAndSources: dispatcher->allSources()) {
    auto c = codeAndSources.first;
    for (const auto &kv: codeAndSources.second) {

#define TRY_TO_MAP(handle, code, description, type, shortname) \
  if (c == handle) {\
    auto y = m->template apply<handle, type >(\
        toTypedDispatchData<handle>(kv.second)->dispatcher()->values());\
    dst->insertValues<type >(handle, kv.first, y);\
  }\

  FOREACH_CHANNEL(TRY_TO_MAP)

#undef TRY_TO_MAP

    }
  }
  return dst;
}

// Visit every channel of a Dispatcher, for side effects.
template <typename Mapper>
void visitDispatcherChannels(const std::shared_ptr<Dispatcher> &dispatcher, Mapper *m) {
  for (const auto &codeAndSources: dispatcher->allSources()) {
    auto c = codeAndSources.first;
    for (const auto &kv: codeAndSources.second) {

#define TRY_TO_MAP(handle, code, shortname, type, description) \
  if (c == handle) {\
    m->template visit<handle, type >(shortname, kv.first, \
        toTypedDispatchData<handle>(kv.second)->dispatcher()->values());\
  }\

  FOREACH_CHANNEL(TRY_TO_MAP)

#undef TRY_TO_MAP

    }
  }
}

template <typename T, DataCode Handle>
std::map<std::string, typename TimedSampleCollection<T>::TimedVector> getTimedVectors(
    const std::map<std::string, DispatchData*> src) {
  std::map<std::string, typename TimedSampleCollection<T>::TimedVector> dst;
  for (const auto &kv: src) {
    dst[kv.first] = toTypedDispatchData<Handle>(kv.second)->dispatcher()->values();
  }
  return dst;
}

template <typename Mapper>
std::shared_ptr<Dispatcher> mergeDispatcherChannels(
    const std::shared_ptr<Dispatcher> &dispatcher, Mapper *m) {
  auto dst = std::make_shared<Dispatcher>();

  for (const auto &codeAndSources: dispatcher->allSources()) {
    auto c = codeAndSources.first;
    auto newSourceName = m->getSourceName(c);
    dst->setSourcePriority(newSourceName, m->getPriority(c));

#define TRY_TO_MERGE(handle, code, shortname, type, description) \
    if (c == handle) { \
      dst->insertValues(c, newSourceName, m->template merge<handle, type>(getTimedVectors<type, handle>(codeAndSources.second))); \
    }

    FOREACH_CHANNEL(TRY_TO_MERGE)

#undef TRY_TO_MERGE

  }
  return dst;
}

/*

TODO: Functions to simplify the Dispatcher

For every code, it should pick the source with the highest priority
std::shared_ptr<Dispatcher> keepBestSources(const std::shared_ptr<Dispatcher> &d);

For every code, it should take the samples from all sources, but remove some samples
so that a sample of lower priority is never closer than 'dur' to a sample of higher priority.
std::shared_ptr<Dispatcher> combineSources(const std::shared_ptr<Dispatcher> &d, const Duration<double> &dur);

*/

}


#endif /* DEVICE_ANEMOBOX_DISPATCHERUTILS_H_ */
