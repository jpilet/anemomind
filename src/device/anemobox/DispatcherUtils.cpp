/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/DispatcherUtils.h>
#include <server/common/logging.h>
#include <server/nautical/AbsoluteOrientation.h>

namespace sail {


namespace {
class DispVisitor {
 public:
  DispVisitor(const Dispatcher *d, std::ostream *dst) : _d(d), _dst(dst) {}

  template <DataCode Code, typename T>
  void visit(const char *shortName, const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {

    auto x = toTypedDispatchData<Code>(raw.get())->dispatcher();

    int prio = _d->sourcePriority(sourceName);

    *_dst << "\n  Channel of type " << shortName << " named "
        << sourceName << " (prio: " << prio << ")"
        << " with " << coll.size() << " samples with "
        << x->listeners().size() << " listeners (";
    for (const auto &y: x->listeners()) {
      *_dst << y << " ";
    }

     *_dst << ")";

  }
 private:
  const Dispatcher *_d;
  std::ostream *_dst;
};

}


std::ostream &operator<<(std::ostream &s, const Dispatcher *d) {
  s << "\nDispatcher:";
  DispVisitor v(d, &s);
  visitDispatcherChannelsConst<DispVisitor>(d, &v);
  return s;
}

int countChannels(const Dispatcher *d) {
  if (d == nullptr) {
    return 0;
  }
  int counter = 0;
  for (const auto &c: d->allSources()) {
    counter += c.second.size();
  }
  return counter;
}

namespace {
  class ValueCounterVisitor {
  public:
    ValueCounterVisitor() : counter(0) {}

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
               const std::shared_ptr<DispatchData> &raw,
               const TimedSampleCollection<T> &coll) {
      counter += coll.size();
    }

    int counter;
  };
}  

int countValues(const Dispatcher *d) {
  ValueCounterVisitor visitor;
  visitDispatcherChannelsConst<ValueCounterVisitor>(d, &visitor);
  return visitor.counter;
}

namespace {

  template <typename T>
  bool isValidColl(const TimedSampleCollection<T> &coll) {
    const auto &samples = coll.samples();
    int n = samples.size();
    int last = n - 1;
    for (int i = 0; i < n; i++) {
      auto x = samples[i];
      if (!isFinite(x.value)) {
        LOG(WARNING) << "Sample " << i << " is not finite";
        return false;
      }
      if (!x.time.defined()) {
        LOG(WARNING) << "Sample " << i << " has undefined time";
        return false;
      }
      if (i < last) {
        auto y = samples[i + 1];
        if (y.time.defined()) {
          if (!(x.time <= y.time)) {
            LOG(WARNING) << "Time stamp at time " << i << " ("
                << x.time.toString() << ") does not precede that of its successor ("
                << y.time.toString() << ")";
            return false;
          }
        }
      }
    }
    return true;
  }

  class ValidVisitor {
  public:
    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
        auto v = isValidColl(coll);
        if (!v) {
          LOG(WARNING) << "Collection of type " << shortName << " from source "
              << sourceName << " is not valid, see explanations above.";
          LOG(WARNING) << "It has " << coll.size() << " samples";
          valid = false;
        }
    }

    bool valid = true;
  private:
  };
}

bool isValid(const Dispatcher *d) {
  ValidVisitor v;
  visitDispatcherChannelsConst<ValidVisitor>(d, &v);
  return v.valid;
}

std::ostream &operator<<(std::ostream &s, const Dispatcher &d) {
  return (s << &d);
}

std::ostream &operator<<(std::ostream &s, const std::shared_ptr<Dispatcher> &d) {
  return (s << d.get());
}


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
      bool closeToEachOther = fabs(b.data.time - x.data.time) < maxMergeDif;
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


void copyPriorities(Dispatcher *src, Dispatcher *dst) {
  for (auto kv: src->sourcePriority()) {
    dst->setSourcePriority(kv.first, kv.second);
  }
}

namespace {
  class FilterVisitor {
   public:
    FilterVisitor(Dispatcher *src,
        DispatcherChannelFilterFunction f, bool includePrios) :
        _src(src), _dst(new Dispatcher()), _f(f), _includePrios(includePrios) {}

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
      bool x = _f(Code, sourceName);
      if (x) {
        _dst->set(Code, sourceName, raw);
      }
      if (x || _includePrios) {
        _dst->setSourcePriority(sourceName, _src->sourcePriority(sourceName));
      }
    }

    Dispatcher *get() {return _dst;}
   private:
    DispatcherChannelFilterFunction _f;
    Dispatcher *_src, *_dst;
    bool _includePrios;
  };
}


std::shared_ptr<Dispatcher> filterChannels(Dispatcher *src,
  DispatcherChannelFilterFunction f, bool includePrios) {
  FilterVisitor v(src, f, includePrios);
  visitDispatcherChannels(src, &v);
  return std::shared_ptr<Dispatcher>(v.get());
}

std::shared_ptr<Dispatcher> shallowCopy(Dispatcher *src) {
  return filterChannels(src, [&](DataCode c, const std::string &srcName) {
    return true;
  }, true);
}

std::map<DataCode, std::map<std::string, std::shared_ptr<DispatchData>>>
  mergeDispatchDataMaps(
      const std::map<DataCode, std::map<std::string,
        std::shared_ptr<DispatchData>>> &a,
      const std::map<DataCode, std::map<std::string,
        std::shared_ptr<DispatchData>>> &b) {
  std::map<DataCode, std::map<std::string, std::shared_ptr<DispatchData>>>
    dst = a;
  for (const auto &codeAndSourceMap: b) {
    auto code = codeAndSourceMap.first;

    auto &dstAtCode = dst[code];
    for (const auto &sourceAndData: codeAndSourceMap.second) {
      dstAtCode[sourceAndData.first] = sourceAndData.second;
    }
  }
  return dst;
}


namespace {
  class PriorityVisitor {
   public:
    PriorityVisitor(Dispatcher *src, Dispatcher *dst) :
        _src(src), _dst(dst) {}

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
      _dst->setSourcePriority(sourceName, _src->sourcePriority(sourceName));
    }
   private:
    Dispatcher *_src, *_dst;
  };
}

std::shared_ptr<Dispatcher> mergeDispatcherWithDispatchDataMap(
    Dispatcher *srcDispatcher,
    const std::map<DataCode, std::map<std::string,
      std::shared_ptr<DispatchData>>> &toAdd) {
  auto merged = mergeDispatchDataMaps(srcDispatcher->allSources(), toAdd);

  auto dst = std::make_shared<Dispatcher>();
  for (const auto &codeAndSources: merged) {
    auto code = codeAndSources.first;
    for (const auto &sourceAndData: codeAndSources.second) {
      dst->set(code, sourceAndData.first, sourceAndData.second);
    }
  }

  // Copy the priorities from the source dispatcher
  PriorityVisitor visitor(srcDispatcher, dst.get());
  visitDispatcherChannels(srcDispatcher, &visitor);

  return dst;
}


namespace {
  const std::map<std::string,
        std::shared_ptr<DispatchData>> *lookUpMap(
            const std::map<DataCode, std::map<std::string,
              std::shared_ptr<DispatchData>>> &A, DataCode c) {
    auto f = A.find(c);
    if (f == A.end()) {
      return nullptr;
    }
    return &(f->second);
  }

  std::shared_ptr<DispatchData> getChannel(
      const std::map<std::string,
        std::shared_ptr<DispatchData>> *a, const std::string &x) {
    auto f = a->find(x);
    if (f == a->end()) {
      return std::shared_ptr<DispatchData>();
    }
    return f->second;
  }

  bool equalSourceMaps(
      const std::map<std::string,
              std::shared_ptr<DispatchData>> *a,
      const std::map<std::string,
        std::shared_ptr<DispatchData>> *b) {
    if (a == nullptr) {
      return b == nullptr;
    } else if (b == nullptr) {
      return false;
    }
    std::set<std::string> allSources;
    for (auto kv: *a) {
      allSources.insert(kv.first);
    }
    for (auto kv: *b) {
      allSources.insert(kv.first);
    }
    for (auto src: allSources) {
      if (getChannel(a, src) != getChannel(b, src)) {
        return false;
      }
    }
    return true;
  }
}

std::set<DataCode> listDataCodesWithDifferences(
    const std::map<DataCode, std::map<std::string,
          std::shared_ptr<DispatchData>>> &A,
    const std::map<DataCode, std::map<std::string,
          std::shared_ptr<DispatchData>>> &B) {
  std::set<DataCode> allDataCodes;
  for (auto kv: A) {
    allDataCodes.insert(kv.first);
  }
  for (auto kv: B) {
    allDataCodes.insert(kv.first);
  }
  std::set<DataCode> difs;
  for (auto code: allDataCodes) {
    if (!equalSourceMaps(lookUpMap(A, code), lookUpMap(B, code))) {
      difs.insert(code);
    }
  }
  return difs;
}


namespace {
  struct ChannelInfo {
   ChannelInfo(const std::string &n, DataCode c) : name(n), code(c) {}

   std::string name;
   DataCode code;

   typedef std::shared_ptr<ChannelInfo> Ptr;
  };

  class ValueToPublish {
   public:
    virtual TimeStamp time() = 0;
    virtual void publish(ReplayDispatcher *dst) = 0;
    virtual ~ValueToPublish() {}

    virtual const ChannelInfo::Ptr &info() const = 0;

    typedef std::shared_ptr<ValueToPublish> Ptr;
  };

  bool before(const ValueToPublish::Ptr &a, const ValueToPublish::Ptr &b) {
    return a->time() < b->time();
  }

  template <typename T>
  class ValueToPublishT : public ValueToPublish {
   public:
    ValueToPublishT(
        const ChannelInfo::Ptr &info,
        const TimedValue<T> &x) : _info(info), _x(x) {}

    TimeStamp time() override {return _x.time;}

    void publish(ReplayDispatcher *dst) {
      dst->publishTimedValue<T>(_info->code, _info->name, _x);
    }

    const ChannelInfo::Ptr &info() const override {return _info;}
   private:
    ChannelInfo::Ptr _info;
    TimedValue<T> _x;
  };

  class ValueCollector {
   public:
    ValueCollector(std::vector<ValueToPublish::Ptr> *dst) : _dst(dst) {}

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {

      auto info = std::make_shared<ChannelInfo>(sourceName, Code);

      for (auto x: coll.samples()) {
        _dst->push_back(std::make_shared<ValueToPublishT<T> >(info, x));
      }
    }

   private:
   std::vector<ValueToPublish::Ptr> *_dst;
  };
}

ReplayDispatcher::ReplayDispatcher() : _counter(0) {}

void ReplayDispatcher::replay(const Dispatcher *src) {
  if (src == nullptr) {
    return;
  }

  std::vector<ValueToPublish::Ptr> allValues;
  ValueCollector collector(&allValues);
  visitDispatcherChannelsConst(src, &collector);
  std::sort(allValues.begin(), allValues.end(), before);
  for (auto x: allValues) {
    x->publish(this);
  }
  finishTimeouts();
}

void ReplayDispatcher::setTimeout(std::function<void()> cb, double delayMS) {
  if (_currentTime.defined()) {
    _counter++;
    auto next = _currentTime + Duration<double>::milliseconds(delayMS);
    _timeouts.insert(Timeout{_counter, next, cb});
  }
}

void ReplayDispatcher::finishTimeouts() {
  for (auto to: _timeouts) {
    to.cb();
  }
  _timeouts = std::set<Timeout>();
}

void ReplayDispatcher::visitTimeouts() {
  std::vector<Timeout> toRemove;
  if (_currentTime.defined()) {
    for (auto to: _timeouts) {
      if (to.time <= _currentTime) {
        to.cb();
        toRemove.push_back(to);
      }
    }
  }
  for (auto to: toRemove) {
    _timeouts.erase(to);
  }
}


}
