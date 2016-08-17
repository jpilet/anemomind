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
  if (!dispatcher) {
    return;
  }
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


// Visit every channel of a Dispatcher, for side effects.
// Const version.
template <typename Mapper>
void visitDispatcherChannelsConst(const Dispatcher *dispatcher, Mapper *m) {
  if (!dispatcher) {
    return;
  }
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

int countChannels(const Dispatcher *d);
int countValues(const Dispatcher *d);

std::ostream &operator<<(std::ostream &s, const Dispatcher &d);
std::ostream &operator<<(std::ostream &s, const Dispatcher *d);
std::ostream &operator<<(std::ostream &s, const std::shared_ptr<Dispatcher> &d);

bool isValid(const Dispatcher *d);

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

static const Duration<double> maxMergeDif = Duration<>::seconds(12.0);

std::shared_ptr<DispatchData> mergeChannels(DataCode code,
    const std::string &srcName,
    const std::map<std::string, int> &priorityMap,
    const std::map<std::string, std::shared_ptr<DispatchData> > &dispatcherMap);

std::shared_ptr<Dispatcher> makeDispatcherFromTextChannel(
    const Dispatcher *d,
    const std::string &srcName);

Array<std::string> getSourceNames(const Dispatcher &d);


typedef std::function<bool(DataCode, const std::string&)>
  DispatcherChannelFilterFunction;

std::shared_ptr<Dispatcher> filterChannels(Dispatcher *src,
  DispatcherChannelFilterFunction f, bool includePrios);
std::shared_ptr<Dispatcher> shallowCopy(Dispatcher *src);


std::map<DataCode, std::map<std::string, std::shared_ptr<DispatchData>>>
  mergeDispatchDataMaps(
      const std::map<DataCode, std::map<std::string,
        std::shared_ptr<DispatchData>>> &a,
      const std::map<DataCode, std::map<std::string,
        std::shared_ptr<DispatchData>>> &b);

std::shared_ptr<Dispatcher> mergeDispatcherWithDispatchDataMap(
    Dispatcher *src,
    const std::map<DataCode, std::map<std::string,
          std::shared_ptr<DispatchData>>> &toAdd);

std::set<DataCode> listDataCodesWithDifferences(
    const std::map<DataCode, std::map<std::string,
          std::shared_ptr<DispatchData>>> &A,
    const std::map<DataCode, std::map<std::string,
          std::shared_ptr<DispatchData>>> &B);

void copyPriorities(const Dispatcher *src, Dispatcher *dst);

typedef std::function<void(const std::shared_ptr<Dispatcher> &,
        DataCode, const std::string &)> ReplayVisitor;

std::shared_ptr<Dispatcher> cloneAndfilterDispatcher(
    Dispatcher *srcDispatcher,
    std::function<bool(DataCode, const std::string&)> filter);


void exportDispatcherToTextFiles(const std::string &filenamePrefix,
    TimeStamp from, TimeStamp to,
    const Dispatcher *d);

class ReplayDispatcher : public Dispatcher {
 public:

  struct Timeout {
    int64_t id;
    TimeStamp time;
    std::function<void()> cb;

    bool operator<(const Timeout &other) const {
      return id < other.id;
    }
  };

  ReplayDispatcher();

   TimeStamp currentTime() override {
     return _currentTime;
   }

   int maxBufferLength() const override {
     return std::numeric_limits<int>::max();
   }

   template <typename T>
     void publishTimedValue(DataCode code, const std::string& source,
                            TimedValue<T> value) {

     // Time can only move forward.
     assert(!_currentTime.defined() || _currentTime <= value.time);

     // This is the only way to advance time of this dispatcher.
     // Consequently, we only need to visit the timeouts here.
     _currentTime = value.time;
     visitTimeouts();
     publishValue<T>(code, source, value.value);
   }

   void replay(const Dispatcher *src);
   void setTimeout(std::function<void()> cb, double delayMS);


   void finishTimeouts();

   const std::set<Timeout> &getTimeouts() const {
     return _timeouts;
   }
 private:
   void visitTimeouts();
   int64_t _counter;
   std::set<Timeout> _timeouts;
   TimeStamp _currentTime;
 };
 
bool saveDispatcher(const std::string& filename, const Dispatcher& nav);

}  // namespace sail

#endif /* DEVICE_ANEMOBOX_DISPATCHERUTILS_H_ */
