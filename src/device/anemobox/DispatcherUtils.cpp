/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/DispatcherUtils.h>

#include <assert.h>
#include <device/anemobox/logger/Logger.h>
#include <server/common/MultiMerge.h>
#include <server/common/logging.h>
#include <server/nautical/AbsoluteOrientation.h>
#include <fstream>
#include <device/anemobox/LazyReplayDispatchData.h>

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

  template <>
  bool isValidColl(const TimedSampleCollection<BinaryEdge> &coll) {
    const BinarySignal &samples = coll.samples();

    if (samples.size() == 0) {
      return true;
    }

    for (int i = 0; i < samples.size(); ++i) {
      if (!samples[i].time.defined()) {
        LOG(WARNING) << "Sample " << i << " has undefined time";
        return false;
      }
    }

    bool result = true;
    if (samples.front().value != BinaryEdge::ToOn) {
      LOG(WARNING) << "First binary edge is ToOff At time "
        << samples.front().time;
      result = false;
    }
    if (samples.back().value != BinaryEdge::ToOff) {
      LOG(WARNING) << "Last binary edge is ToOn At time "
        << samples.back().time;
      result = false;
    }
    for (int i = 0; (i + 1) < samples.size(); i += 2) {
      if (samples[i].value == samples[i+1].value) {
        LOG(WARNING) << "Two consecutive " << (samples[i].value == BinaryEdge::ToOn ?
                                               "ToOn" : "ToOff") << " samples "
          " at time " << samples[i].time;
        result = false;
      }
    }

    return result;
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


void copyPriorities(const Dispatcher *src, Dispatcher *dst) {
  for (auto kv: src->sourcePriority()) {
    dst->setSourcePriority(kv.first, kv.second);
  }
}

namespace {
  class FilterVisitor {
   public:
    FilterVisitor(Dispatcher *src,
        DispatcherChannelFilterFunction f, bool includePrios,
        DispatcherChannelMapperFunction transform) :
        _src(src), _dst(new Dispatcher()), _f(f),
        _includePrios(includePrios),
        _transform(transform) {}

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
      bool x = _f(Code, sourceName);
      if (x) {
        _dst->set(Code, sourceName, _transform(Code, sourceName, raw));
      }
      if (x || _includePrios) {
        _dst->setSourcePriority(sourceName, _src->sourcePriority(sourceName));
      }
    }

    Dispatcher *get() {return _dst;}
   private:
    DispatcherChannelFilterFunction _f;
    DispatcherChannelMapperFunction _transform;
    Dispatcher *_src, *_dst;
    bool _includePrios;
  };
}


std::shared_ptr<Dispatcher> filterChannels(Dispatcher *src,
  DispatcherChannelFilterFunction f, bool includePrios,
  DispatcherChannelMapperFunction tr) {
  FilterVisitor v(src, f, includePrios, tr);
  visitDispatcherChannels(src, &v);
  return std::shared_ptr<Dispatcher>(v.get());
}

std::shared_ptr<Dispatcher> shallowCopy(Dispatcher *src) {
  return filterChannels(src, [&](DataCode c, const std::string &srcName) {
    return true;
  }, true);
}

bool constantlyTrueForCodeAndSource(DataCode, const std::string& ) {
  return true;
}

bool timeInRange(TimeStamp x, TimeStamp lower, TimeStamp upper) {
  return (lower.undefined() || (lower <= x))
      && (upper.undefined() || (x <= upper));
}

template <DataCode code>
std::shared_ptr<DispatchData> filterByTimeForType(
    const std::string& src,
    TimeStamp from,
    TimeStamp to,
    const std::shared_ptr<DispatchData>& data0) {
  typedef typename TypeForCode<code>::type T;
  TypedDispatchData<T>* data = toTypedDispatchData<code>(data0.get());
  const auto& samples = data->dispatcher()->values().samples();

  auto dst = new TypedDispatchDataReal<T>(code, src, nullptr, samples.size());
  for (const auto& x: samples) {
    if (timeInRange(x.time, from, to)) {
      dst->dispatcher()->mutableValues()->append(x);
    }
  }

  return std::shared_ptr<DispatchData>(dst);
}

DispatcherChannelMapperFunction filterByTime(TimeStamp from, TimeStamp to) {
  return [from,to](
      DataCode code,
      const std::string& src,
      const std::shared_ptr<DispatchData>& data) {
#define FILTER_BY_TIME_FOR_CODE(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  if (code == HANDLE) {return filterByTimeForType<HANDLE>(src, from, to, data);}
    FOREACH_CHANNEL(FILTER_BY_TIME_FOR_CODE)
#undef FILTER_BY_TIME_FOR_CODE
    return std::shared_ptr<DispatchData>();
  };
}

std::shared_ptr<Dispatcher> cropDispatcher(
    Dispatcher *src,
      TimeStamp from, TimeStamp to) {
  return filterChannels(src,
      &constantlyTrueForCodeAndSource, true,
      filterByTime(from, to));
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

std::shared_ptr<Dispatcher> cloneAndfilterDispatcher(
    Dispatcher *srcDispatcher,
    std::function<bool(DataCode, const std::string&)> filter) {

  std::shared_ptr<Dispatcher> dst = std::make_shared<Dispatcher>();
  for (const auto &codeAndSources: srcDispatcher->allSources()) {
    auto code = codeAndSources.first;
    for (const auto &sourceAndData: codeAndSources.second) {
      if (filter(code, sourceAndData.first)) {
        dst->set(code, sourceAndData.first, sourceAndData.second);
      }
    }
  }

  copyPriorities(srcDispatcher, dst.get());

  return dst;
}


namespace {
  template <typename T>
  struct ValueExporter {
    static const bool active = false;

    double toNumber(const T &x) {
      return 0.0;
    }

    static const char *getUnitLabel() {return nullptr;}
  };

  template <>
  struct ValueExporter<Velocity<double> > {

    static const bool active = true;

    static double toNumber(const Velocity<double> &v) {
      return v.metersPerSecond();
    }

    static const char *getUnitLabel() {return "meters per second";}
  };

  template <>
  struct ValueExporter<Angle<double> > {

    static const bool active = true;

    static double toNumber(const Angle<double> &x) {
      return x.radians();
    }

    static const char *getUnitLabel() {return "radians";}
  };

  Duration<double> exportSamplingPeriod = Duration<double>::seconds(1.0);

  template <typename T>
  Optional<T> getValueCloseTo(const TimedSampleCollection<T> &coll, TimeStamp t) {
    auto x0 = coll.nearestTimedValue(t);
    if (x0.defined()) {
      auto tv = x0.get();
      if (fabs(tv.time - t) < Duration<double>::seconds(8.0)) {
        return tv.value;
      }
    }
    return Optional<T>();
  }


  namespace {
    class ExportVisitor {
     public:
      ExportVisitor(const std::string &filename,
          TimeStamp fromTime, TimeStamp toTime) :
        _shortNameFile(filename + "_shortnames.txt"),
        _sourceNameFile(filename + "_sourcenames.txt"),
        _dataFile(filename + "_data.txt"),
        _unitFile(filename + "_units.txt"), _fromTime(fromTime), _toTime(toTime) {

        // The first row is the time.
        _shortNameFile << "TIME\n";
        _sourceNameFile << "ExportVisitor\n";
        _unitFile << "Seconds\n";
        for (auto t = _fromTime; t < _toTime; t = t + exportSamplingPeriod) {
          _dataFile << t.toSecondsSince1970() << " ";
        }
        _dataFile << std::endl;
      }

      template <DataCode Code, typename T>
      void visit(const char *shortName, const std::string &sourceName,
        const std::shared_ptr<DispatchData> &raw,
        const TimedSampleCollection<T> &coll) {
        ValueExporter<T> exporter;
        if (exporter.active) {
          _unitFile << exporter.getUnitLabel() << "\n";
          _shortNameFile << shortName << "\n";
          _sourceNameFile << sourceName << "\n";
          for (auto t = _fromTime; t < _toTime; t = t + exportSamplingPeriod) {
            auto x0 = getValueCloseTo(coll, t);
            if (x0.defined()) {
              _dataFile << exporter.toNumber(x0.get()) << " ";
            } else {
              _dataFile << "NAN ";
            }
          }
          _dataFile << std::endl;
        }
      }
     private:
      std::ofstream _shortNameFile, _sourceNameFile, _dataFile, _unitFile;
      TimeStamp _fromTime, _toTime;
    };
  }
}

void exportDispatcherToTextFiles(const std::string &filenamePrefix,
    TimeStamp from, TimeStamp to,
    const Dispatcher *d) {
  ExportVisitor visitor(filenamePrefix, from, to);
  visitDispatcherChannelsConst(d, &visitor);
}

std::function<TimeStamp(TimeStamp)> roundOffToBin(
    Duration<double> dur) {
  int64_t durMillis = int64_t(dur.milliseconds());
  return [durMillis](TimeStamp t) {
    return TimeStamp::fromMilliSecondsSince1970(
        durMillis*(t.toMilliSecondsSince1970()/durMillis));
  };
}

namespace {

  struct SummarizeVisitor {
    std::map<TimeStamp,
      std::map<std::pair<DataCode,
        std::string>, int>> dst;

    std::function<TimeStamp(TimeStamp)> roundOff;

    template <DataCode Code, typename T>
      void visit(const char *shortName, const std::string &sourceName,
        const std::shared_ptr<DispatchData> &raw,
        const TimedSampleCollection<T> &coll) {
      TimeStamp last;
      int* n = nullptr;
      for (const auto& x: coll.samples()) {
        auto at = roundOff(x.time);
        if (!(at == last)) {
          auto& bin = dst[at];
          last = at;
          n = &(bin[{Code, sourceName}]);
        }
        (*n)++;
      }
    }
  };

}

std::map<TimeStamp, std::map<std::pair<DataCode, std::string>, int>>
  summarizeDispatcherOverTime(
      const Dispatcher* d,
      std::function<TimeStamp(TimeStamp)> roundOff) {
  SummarizeVisitor v;
  v.roundOff = roundOff;
  visitDispatcherChannelsConst<SummarizeVisitor>(d, &v);
  return v.dst;
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

template <typename T>
class DispatcherStream : public SortedStream<TimeStamp> {
 public:
  DispatcherStream(TypedDispatchData<T> *dispatchData,
                   ReplayDispatcher *destination)
    : _dispatchData(dispatchData),
    _it(dispatchData->dispatcher()->values().samples().begin()),
    _destination(destination) { }

  virtual TimeStamp value() const { return _it->time; }
  virtual bool next() {
    assert(!end());
    _destination->publishTimedValue<T>(_dispatchData->dataCode(),
                                       _dispatchData->source(),
                                       *_it);
    ++_it;
    return end();
  }
  virtual bool end() const {
    return _it == _dispatchData->dispatcher()->values().samples().end();
  }

 private:
  TypedDispatchData<T> *_dispatchData;
  typename TimedSampleCollection<T>::TimedVector::const_iterator _it;
  ReplayDispatcher *_destination;
};

class DispatchDataMerger : public DispatchDataVisitor {
 public:
  DispatchDataMerger(ReplayDispatcher *destination)
    : _destination(destination) { }
  template<typename T>
  void addStream(TypedDispatchData<T> *d) {
    auto s = std::make_shared<DispatcherStream<T>>(d, _destination);
    _streams.push_back(s);
  }

  virtual void run(DispatchAngleData *d) { addStream(d); }
  virtual void run(DispatchVelocityData *d) { addStream(d); }
  virtual void run(DispatchLengthData *d) { addStream(d); }
  virtual void run(DispatchGeoPosData *d) { addStream(d); }
  virtual void run(DispatchTimeStampData *d) { addStream(d); }
  virtual void run(DispatchAbsoluteOrientationData *d) { addStream(d); }
  virtual void run(DispatchBinaryEdge *d) { addStream(d); }

  void merge() {
    MultiMerge<TimeStamp> merger;
    for (auto s : _streams) {
      merger.addStream(s.get());
    }
    while (!merger.end()) {
      // Publish occurs in the "next" method of DispatcherStream,
      // called in the right order by _merger.
      merger.next();
    }
  }
 private:
  // This vector is used to delete all the allocated SortedStreams
  // when destructing the DispatchDataMerger object.
  std::vector<std::shared_ptr<SortedStream<TimeStamp>>> _streams;

  ReplayDispatcher *_destination;
};

}  // namespace 

ReplayDispatcher::ReplayDispatcher() : _counter(0) {}

void ReplayDispatcher::replay(const Dispatcher *src) {
  if (src == nullptr) {
    return;
  }
  _replayingFrom = src;

  copyPriorities(src, this);

  DispatchDataMerger merger(this);

  for (auto code : src->allSources()) {
    for (auto source : code.second) {
      source.second->visit(&merger);
    }
  }

  merger.merge();
  finishTimeouts();

  finalizeLazyReplay();
}

void ReplayDispatcher::finalizeLazyReplay() {
  for (auto x: _toFinalize) {
    finalizeLazyReplayDispatchData(x);
  }
  _toFinalize = std::vector<DispatchData*>();
  _replayingFrom = nullptr;
}

void ReplayDispatcher::setTimeout(std::function<void()> cb, double delayMS) {
  if (_currentTime.defined()) {
    _counter++;
    auto next = _currentTime + Duration<double>::milliseconds(delayMS);
    _timeouts.insert(Timeout{_counter, next, cb});
  }
}

DispatchData* ReplayDispatcher::createNewCustomDispatchData(
      DataCode code, const std::string& src, int unusedSize) {

  // Ignore the provided size...
  (void)unusedSize;
  // ... and provide our own, short-term size.
  // For all LazyReplayDispatchData, once we call
  // finalize, the size will be arbitrarily large.
  int shortSize = 30;

  if (_replayingFrom) {
    auto fCode = _replayingFrom->allSources().find(code);
    if (fCode != _replayingFrom->allSources().end()) {
      auto f = fCode->second.find(src);
      if (f != fCode->second.end()) {
        auto x = makeLazyReplayDispatchData(this, shortSize, f->second);
        _toFinalize.push_back(x);
        return x;
      }
    }
  }
  return nullptr;
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

bool saveDispatcher(const std::string& filename, const Dispatcher& nav) {
  ReplayDispatcher replay;
  Logger logger(&replay);

  replay.replay(&nav);

  LogFile logged;
  logger.flushTo(&logged);

  return Logger::save(filename, logged);
}


}
