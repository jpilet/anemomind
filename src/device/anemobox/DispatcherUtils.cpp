/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/DispatcherUtils.h>

namespace sail {

namespace {

  template <typename T>
  class PrioritizedDispatchData {
   public:
    typedef typename TimedSampleCollection<T>::TimedVector TimedVector;
    typedef typename TimedVector::const_iterator Iterator;

    PrioritizedDispatchData(int priority, Iterator b, Iterator e) :
      _priority(priority), _begin(b), _end(e) {
      assert(_begin <= _end);
    }

    bool empty() const {
      return _begin == _end;
    }

    int size() const {
      return _end - _begin;
    }

    bool operator<(const PrioritizedDispatchData &other) const {
      return _priority > other._priority;
    }

    const TimedValue<T> &front() const {
      assert(!empty());
      return *_begin;
    }

    void popFront() {
      assert(!empty());
      _begin++;
    }

    int priority() const {
      return _priority;
    }
   private:
    int _priority;
    Iterator _begin, _end;
  };

  template <DataCode Code>
  const typename TimedSampleCollection<typename TypeForCode<Code>::type>::TimedVector
      &getSamples(DispatchData *d) {
    return toTypedDispatchData<Code>(d)->dispatcher()->values().samples();
  }

  template <DataCode Code>
  int getSampleCount(DispatchData *d) {
    return getSamples<Code>(d).size();
  }

  template <typename T>
  bool descending(const std::vector<PrioritizedDispatchData<T> > &X) {
    int n = X.size() - 1;
    for (int i = 0; i < n; i++) {
      if (!(X[i].priority() >= X[i+1].priority())) {
        return false;
      }
    }
    return true;
  }

  template <DataCode Code>
  std::vector<PrioritizedDispatchData<typename TypeForCode<Code>::type> > getPrioritizedDispatchData(
      const std::map<std::string, int> &sourcePriority,
      const std::map<std::string, std::shared_ptr<DispatchData> > &dispatchData) {
    typedef typename TypeForCode<Code>::type ElementType;
    std::vector<PrioritizedDispatchData<ElementType> > dst;
    dst.reserve(dispatchData.size());
    for (const auto &kv: dispatchData) {
      const auto &samples = getSamples<Code>(kv.second.get());
      dst.push_back(PrioritizedDispatchData<ElementType>{
        getSourcePriority(sourcePriority, kv.first),
        samples.begin(), samples.end()});
    }
    std::sort(dst.begin(), dst.end());
    assert(descending(dst));
    return dst;
  }

  template <typename T>
  struct PrioritizedSample {
    int priority;
    TimedValue<T> data;
  };

  template <typename T>
  typename TimedSampleCollection<T>::TimedVector toTimedVector(
      const std::vector<PrioritizedSample<T> > &src) {
    typename TimedSampleCollection<T>::TimedVector dst;
    for (auto x: src) {
      dst.push_back(x.data);
    }
    return dst;
  }

  template <typename T>
  int getEarliestIndex(
      const std::vector<PrioritizedDispatchData<T> > &prioritized) {
    int index = -1;
    sail::TimeStamp earliest;
    int n = prioritized.size();
    for (int i = 0; i < n; i++) {
      const auto &p = prioritized[i];
      if (!p.empty() && (earliest.undefined() || p.front().time < earliest)) {
        earliest = p.front().time;
        index = i;
      }
    }
    return index;
  }

  template <typename T>
  bool isEmpty(const std::vector<PrioritizedDispatchData<T> > &prioritized) {
    return getEarliestIndex(prioritized) == -1;
  }

  template <typename T>
  PrioritizedSample<T> popWithIndex(std::vector<PrioritizedDispatchData<T> > *prioritized, int index) {
    assert(index != -1);
    auto &p = (*prioritized)[index];
    auto x = p.front();
    p.popFront();
    return PrioritizedSample<T>{p.priority(), x};
  }

  template <typename T>
  void addSample(std::vector<PrioritizedSample<T> > *dst, const PrioritizedSample<T> &x) {
    while (!dst->empty()) {
      auto &b = dst->back();
      bool closeToEachOther = std::abs((b.data.time - x.data.time).seconds()) < maxMergeDifSeconds;
      if (closeToEachOther) {
        if (x.priority == b.priority) {
          break;
        } else if (x.priority < b.priority) {
          return;
        } else { // x.priority > b.priority must be true
          dst->pop_back();
        }
      } else {
        break;
      }
    }
    dst->push_back(x);
  }

  template <typename T>
  typename TimedSampleCollection<T>::TimedVector mergePrioritized(
      std::vector<PrioritizedDispatchData<T> > *prioritized) {
    std::vector<PrioritizedSample<T> >  dst;
    dst.reserve((*prioritized)[0].size());
    while (true) {
      int index = getEarliestIndex(*prioritized);
      if (index == -1) {
        break;
      }
      addSample(&dst, popWithIndex(prioritized, index));
    }
    return toTimedVector(dst);
  }
}

template <DataCode Code>
std::shared_ptr<DispatchData> mergeChannelsSub(
    const std::string &srcName,
    const std::map<std::string, int> &priorityMap,
    const std::map<std::string, std::shared_ptr<DispatchData> > &dispatcherMap) {
  int n = dispatcherMap.size();
  typedef typename TypeForCode<Code>::type T;
  if (n == 0) {
    return std::shared_ptr<DispatchData>();
  } else if (n == 1) {
    return dispatcherMap.begin()->second;
  } else {
    auto prio = getPrioritizedDispatchData<Code>(
                    priorityMap, dispatcherMap);
    return std::shared_ptr<DispatchData>(
        makeDispatchDataFromSamples<Code>(srcName,
            mergePrioritized<T>(&prio)));
  }
}

std::shared_ptr<DispatchData> mergeChannels(DataCode code,
    const std::string &srcName,
    const std::map<std::string, int> &priorityMap,
    const std::map<std::string, std::shared_ptr<DispatchData> > &dispatcherMap) {
  switch (code) {
#define MERGE(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    case HANDLE: {return mergeChannelsSub<HANDLE>(srcName, priorityMap, dispatcherMap);}
  FOREACH_CHANNEL(MERGE)
#undef MERGE
  default: return std::shared_ptr<DispatchData>();
  }
  return std::shared_ptr<DispatchData>();
}


}
