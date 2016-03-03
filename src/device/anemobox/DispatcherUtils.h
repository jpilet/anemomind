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
        toTypedDispatchData<handle>(kv.second.get())->dispatcher()->values());\
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
        toTypedDispatchData<handle>(kv.second.get())->dispatcher()->values());\
  }\

  FOREACH_CHANNEL(TRY_TO_MAP)

#undef TRY_TO_MAP

    }
  }
}

// See also: toTypedDispatchData, which goes almost in opposite direction.
template <DataCode Code>
std::shared_ptr<DispatchData> makeDispatchDataFromSamples(
    const std::string &srcName,
    const typename TimedSampleCollection<typename TypeForCode<Code>::type >::TimedVector &values) {
  static Clock theClock;
  auto data = new TypedDispatchDataReal<typename TypeForCode<Code>::type >(
      Code, srcName, &theClock, values.size());
  data->dispatcher()->insert(values);
  return std::shared_ptr<DispatchData>(data);
}

static const double maxMergeDifSeconds = 12.0;

std::shared_ptr<DispatchData> mergeChannels(DataCode code,
    const std::string &srcName,
    const std::map<std::string, int> &priorityMap,
    const std::map<std::string, std::shared_ptr<DispatchData> > &dispatcherMap);

}

#endif /* DEVICE_ANEMOBOX_DISPATCHERUTILS_H_ */
