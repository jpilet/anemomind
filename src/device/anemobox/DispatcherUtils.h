/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef DEVICE_ANEMOBOX_DISPATCHERUTILS_H_
#define DEVICE_ANEMOBOX_DISPATCHERUTILS_H_

#include <memory>
#include <device/anemobox/Dispatcher.h>

namespace sail {

/*

// TODO: Rename it

class VisitorTemplate {
 public:

  template <DataCode Code, typename T>
  void visit(const char *shortName, const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {

      // TODO: write your code here

  }
};

*/

// Visit every channel of a Dispatcher, for side effects.
template <typename Mapper>
void visitDispatcherChannels(Dispatcher *dispatcher, Mapper *m) {
  for (const auto &codeAndSources: dispatcher->allSources()) {
    auto c = codeAndSources.first;
    for (const auto &kv: codeAndSources.second) {

#define TRY_TO_MAP(handle, code, shortname, type, description) \
  if (c == handle) {\
    auto raw = kv.second;\
    m->template visit<handle, type >(shortname, kv.first, \
        raw, \
        toTypedDispatchData<handle>(raw.get())->dispatcher()->values());\
  }\

  FOREACH_CHANNEL(TRY_TO_MAP)

#undef TRY_TO_MAP

    }
  }
}

int countChannels(Dispatcher *d);

std::ostream &operator<<(std::ostream &s, Dispatcher &d);
std::ostream &operator<<(std::ostream &s, Dispatcher *d);
std::ostream &operator<<(std::ostream &s, const std::shared_ptr<Dispatcher> &d);

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

Array<std::string> getSourceNames(const Dispatcher &d);


typedef std::function<bool(DataCode, const std::string&)>
  DispatcherChannelFilterFunction;

std::shared_ptr<Dispatcher> filterChannels(Dispatcher *src,
  DispatcherChannelFilterFunction f, bool includePrios);
std::shared_ptr<Dispatcher> shallowCopy(Dispatcher *src);

typedef std::function<void(const std::shared_ptr<Dispatcher> &,
        DataCode, const std::string &)> ReplayVisitor;

// Populates a new dispatcher with the data of an existing
// dispatcher, by adding one sample at a time. Whenever a new sample
// is added and the visitor was called at least a 'period' ago,
// the visitor is called (again).
std::shared_ptr<Dispatcher> replay(
    Dispatcher *src,
    const ReplayVisitor &visitor,
    Duration<double> period);

}

#endif /* DEVICE_ANEMOBOX_DISPATCHERUTILS_H_ */
