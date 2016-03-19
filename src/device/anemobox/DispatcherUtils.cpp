/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/DispatcherUtils.h>

namespace sail {


namespace {
class Visitor {
 public:
  Visitor(Dispatcher *d, std::ostream *dst) : _d(d), _dst(dst) {}

  template <DataCode Code, typename T>
  void visit(const char *shortName, const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {

    *_dst << "\n  Channel of type " << shortName << " named "
        << sourceName << " with " << coll.size() << " samples.";

  }
 private:
  Dispatcher *_d;
  std::ostream *_dst;
};

}


std::ostream &operator<<(std::ostream &s, Dispatcher *d) {
  s << "\nDispatcher:";
  Visitor v(d, &s);
  visitDispatcherChannels<Visitor>(d, &v);
  return s;
}

int countChannels(Dispatcher *d) {
  if (d == nullptr) {
    return 0;
  }
  int counter = 0;
  for (const auto &c: d->allSources()) {
    counter += c.second.size();
  }
  return counter;
}

std::ostream &operator<<(std::ostream &s, Dispatcher &d) {
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

namespace {
  class FixedTimeDispatcher : public Dispatcher {
   public:
    TimeStamp currentTime() override {
      return _currentTime;
    }

    template <typename T>
    void publishTimedValue(DataCode code, const std::string& source,
        TimedValue<T> value) {
      _currentTime = value.time;
      publishValue<T>(code, source, value.value);
    }
   private:
    TimeStamp _currentTime;
  };

  struct ChannelInfo {
   ChannelInfo(const std::string &n, DataCode c) : name(n), code(c) {}

   std::string name;
   DataCode code;

   typedef std::shared_ptr<ChannelInfo> Ptr;
  };

  class ValueToPublish {
   public:
    virtual TimeStamp time() = 0;
    virtual void publish(FixedTimeDispatcher *dst) = 0;
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

    void publish(FixedTimeDispatcher *dst) {
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

  void stepSampleBySample(const std::vector<ValueToPublish::Ptr> &allValues,
      std::shared_ptr<Dispatcher> sd,
      FixedTimeDispatcher *d,
      const ReplayVisitor &visitor,
      Duration<double> period) {
    TimeStamp nextTime;
    for (int i = 0; i < allValues.size(); i++) {
      ValueToPublish::Ptr x = allValues[i];
      x->publish(d);
      if (!nextTime.defined() || nextTime <= d->currentTime()) {
        nextTime = d->currentTime() + period;
        visitor(sd, x->info()->code, x->info()->name);
      }
    }
  }
}


std::shared_ptr<Dispatcher> replay(
    Dispatcher *src,
    const ReplayVisitor &visitor,
    Duration<double> period) {
  if (src == nullptr) {
    return nullptr;
  }

  std::vector<ValueToPublish::Ptr> allValues;
  ValueCollector collector(&allValues);
  visitDispatcherChannels(src, &collector);
  std::sort(allValues.begin(), allValues.end(), before);

  if (allValues.empty()) {
    return nullptr;
  }

  auto *d = new FixedTimeDispatcher();
  std::shared_ptr<Dispatcher> sd(d);
  stepSampleBySample(allValues, sd, d, visitor, period);
  return sd;
}



}
