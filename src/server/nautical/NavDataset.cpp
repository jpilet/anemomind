/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <assert.h>
#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/DispatcherUtils.h>
#include <device/anemobox/Sources.h>
#include <server/common/logging.h>
#include <server/nautical/NavDataset.h>

namespace sail {

namespace {
  bool before(const TimeStamp &a, const TimeStamp &b, bool whenUndefined) {
    if (a.undefined() || b.undefined()) { // => Assume a means -infinity
      return whenUndefined;
    }
    return a < b;
  }

  bool beforeOrEqual(const TimeStamp &a, const TimeStamp &b, bool whenUndefined) {
    if (a.undefined() || b.undefined()) { // => Assume a means -infinity
      return whenUndefined;
    }
    return a <= b;
  }

  TimeStamp minTime(TimeStamp a, TimeStamp b) {
    if (a.undefined()) {
      return b;
    } else if (b.undefined()) {
      return a;
    }
    return std::min(a, b);
  }

  TimeStamp maxTime(TimeStamp a, TimeStamp b) {
    if (a.undefined()) {
      return b;
    } else if (b.undefined()) {
      return a;
    }
    return std::max(a, b);
  }
}

NavDataset::NavDataset(const std::shared_ptr<Dispatcher> &dispatcher,
    const std::shared_ptr<std::map<DataCode, std::shared_ptr<DispatchData> > > &merged,
    TimeStamp a, TimeStamp b) :
  _dispatcher(dispatcher), _lowerBound(a), _upperBound(b),
  _merged(merged) {
  assert(merged);
  assert(dispatcher);
  assert(beforeOrEqual(_lowerBound, _upperBound, true));

  for (auto channel : dispatcher->allSources()) {
    for (auto source : channel.second) {
      assert(classify(source.first) != SourceOrigin::UNKNOWN);
    }
  }
}

NavDataset NavDataset::slice(TimeStamp a, TimeStamp b) const {
  assert(beforeOrEqual(a, b, true));
  assert(beforeOrEqual(_lowerBound, a, true));
  assert(beforeOrEqual(b, _upperBound, true));
  return NavDataset(_dispatcher, _merged, a, b);
}

NavDataset NavDataset::sliceFrom(TimeStamp ts) const {
  return slice(ts, _upperBound);
}

NavDataset NavDataset::sliceTo(TimeStamp ts) const {
  return slice(_lowerBound, ts);
}

NavDataset NavDataset::sliceFirst(const Duration<double> &dur) const {
  assert(_lowerBound.defined());
  return slice(_lowerBound, _lowerBound + dur);
}

NavDataset NavDataset::sliceLast(const Duration<double> &dur) const {
  assert(_upperBound.defined());
  return slice(_upperBound - dur, _upperBound);
}

namespace {
  std::shared_ptr<std::map<DataCode, std::shared_ptr<DispatchData>>>
    resetMergedChannels(
        const std::shared_ptr<std::map<DataCode, std::shared_ptr<DispatchData>>> &src,
        const std::set<DataCode> &toReset) {
    if (!bool(src)) {
      return std::make_shared<std::map<DataCode, std::shared_ptr<DispatchData>>>();
    }

    if (toReset.empty()) {
      return src;
    }

    auto copy = std::make_shared<std::map<DataCode,
        std::shared_ptr<DispatchData>>>(*src);

    for (auto x: toReset) {
      copy->erase(x);
    }
    return copy;
  }

  void setSuperiorPriorities(std::shared_ptr<Dispatcher> d,
      const std::map<DataCode, std::map<std::string,
          std::shared_ptr<DispatchData>>> &toAdd) {
    auto m = d->maxPriority() + 1;
    for (auto codeAndSrcMap: toAdd) {
      for (auto srcAndData: codeAndSrcMap.second) {
        d->setSourcePriority(srcAndData.first, m);
      }
    }
  }
}

NavDataset NavDataset::overrideChannels(const std::map<DataCode, std::map<std::string,
    std::shared_ptr<DispatchData>>> &toAdd) const {
  auto init = isDefaultConstructed()?
      std::make_shared<Dispatcher>()
             : _dispatcher;

  auto d1 = mergeDispatcherWithDispatchDataMap(
        init.get(), toAdd);

  setSuperiorPriorities(d1, toAdd);

  auto dirty = listDataCodesWithDifferences(init->allSources(), d1->allSources());
  return NavDataset(d1, resetMergedChannels(_merged, dirty), _lowerBound, _upperBound);
}

NavDataset NavDataset::overrideChannels(
    const std::string &srcName,
      const std::map<DataCode,
      std::shared_ptr<DispatchData>> &toAdd) const {
  std::map<DataCode, std::map<std::string,
      std::shared_ptr<DispatchData>>> toAdd2;
  for (auto kv: toAdd) {
    toAdd2[kv.first][srcName] = kv.second;
  }
  return overrideChannels(toAdd2);
}

bool NavDataset::hasLowerBound() const {
  return _lowerBound.defined();
}

bool NavDataset::hasUpperBound() const {
  return _upperBound.defined();
}

bool NavDataset::isBounded() const {
  return hasLowerBound() && hasUpperBound();
}

TimeStamp NavDataset::lowerBound() const {
  return _lowerBound;
}

TimeStamp NavDataset::upperBound() const {
  return _upperBound;
}

Duration<double> NavDataset::duration() const {
  assert(isBounded());
  return _upperBound - _lowerBound;
}

namespace {
  class BoundVisitor {
   public:

    template <DataCode Code, typename T>
        void visit(const char *shortName, const std::string &sourceName,
            const std::shared_ptr<DispatchData> &raw,
            const TimedSampleCollection<T> &coll) {
      if (!coll.empty()) {
        auto lower = coll.samples().front().time;
        auto upper = coll.samples().back().time;
        _lowerBound = minTime(_lowerBound, lower);
        _upperBound = maxTime(_upperBound, upper);
      }
    }

    TimeStamp lowerBound() const {
      return _lowerBound;
    }

    TimeStamp upperBound() const {
      return _upperBound;
    }
   private:
    TimeStamp _lowerBound, _upperBound;
  };
}

NavDataset NavDataset::fitBounds() const {
  BoundVisitor visitor;
  visitDispatcherChannels(_dispatcher.get(), &visitor);
  return NavDataset(_dispatcher, _merged, visitor.lowerBound(), visitor.upperBound());
}

void NavDataset::outputSummary(std::ostream *dst) const {
  std::stringstream ss;
  *dst << "\n\nNavDataset summary:";
  *dst << "\nMerged channels:";
#define DISP_CHANNEL(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  { \
    auto x = getMergedSamples<HANDLE>(); if (x == nullptr) { \
      ss << #HANDLE << " "; \
    } else { \
      *dst << "\n  * Channel " << #HANDLE << " (" << DESCRIPTION << ") with " << x->size() << " values"; \
    } \
  }
  FOREACH_CHANNEL(DISP_CHANNEL)
#undef DISP_CHANNEL

  *dst << "\nNavDataset internal dispatcher: ";
  *dst << _dispatcher;

  *dst << "\n\n  * The following channels are not part of this dataset: " << ss.str() << "\n" << std::endl;
}

bool NavDataset::isDefaultConstructed() const {
  return !_dispatcher;
}

std::ostream &operator<<(std::ostream &s, const NavDataset &ds) {
  ds.outputSummary(&s);
  return s;
}

const std::shared_ptr<DispatchData> &getMergedDispatchData(
  DataCode code,
  const std::shared_ptr<std::map<DataCode, std::shared_ptr<DispatchData>>> &merged,
  const std::shared_ptr<Dispatcher> &dispatcher) {
  assert(bool(merged));
  {
    auto found = merged->find(code);
    if (found != merged->end()) {
      return found->second;
    }
  }

  auto &dst = (*merged)[code];
  if (!bool(dispatcher)) {
    return dst;
  }

  const auto &allSources = dispatcher->allSources();
  auto found = allSources.find(code);

  if (found != allSources.end()) {
    dst = mergeChannels(
        code, "merged", dispatcher->sourcePriority(),
        found->second);
  }
  return dst;
}

NavDataset NavDataset::stripChannel(DataCode code) const {
  if (_dispatcher == nullptr) {
    return NavDataset();
  }

  return NavDataset(
      cloneAndfilterDispatcher(
          _dispatcher.get(),
          [code](DataCode testedCode, const std::string&) { return testedCode != code; }
          ),
      std::make_shared<std::map<DataCode, std::shared_ptr<DispatchData> > >(),
      _lowerBound,
      _upperBound);
}

}
