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

NavDataset::NavDataset(
    const std::shared_ptr<Dispatcher> &dispatcher,
    TimeStamp a, TimeStamp b,
    std::map<DataCode, std::shared_ptr<DispatchData>> activeSource) :
      _dispatcher(dispatcher), _lowerBound(a), _upperBound(b),
      _activeSource(activeSource) {
  assert(dispatcher);
  assert(beforeOrEqual(_lowerBound, _upperBound, true));

  for (auto channel : dispatcher->allSources()) {
    for (auto source : channel.second) {
      classify(source.first);
      //LOG_IF(WARNING, classify(source.first) == SourceOrigin::UNKNOWN) << "Working with unknown source: " << source.first;
    }
  }
}

NavDataset NavDataset::slice(TimeStamp a, TimeStamp b) const {
  assert(beforeOrEqual(a, b, true));
  assert(beforeOrEqual(_lowerBound, a, true));
  assert(beforeOrEqual(b, _upperBound, true));
  return NavDataset(_dispatcher, a, b, _activeSource);
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
  return NavDataset(_dispatcher,
                    visitor.lowerBound(), visitor.upperBound(),
                    _activeSource);
}

std::string NavDataset::boundsAsString() const {
  BoundVisitor visitor;
  visitDispatcherChannels(_dispatcher.get(), &visitor);
  std::string result = visitor.lowerBound().toString()
    + " - " + visitor.upperBound().toString();
  if (_lowerBound.defined()) {
    result += " start selection: " + _lowerBound.toString();
  }
  if (_upperBound.defined()) {
    result += " end selection: " + _upperBound.toString();
  }
  return result;
}

void NavDataset::outputSummary(std::ostream *dst) const {
  std::stringstream ss;
  *dst << "\n\nNavDataset summary: " << boundsAsString();
  *dst << "\nMerged channels:";
#define DISP_CHANNEL(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  { \
    std::vector<std::string> sources = sourcesForChannel(HANDLE); \
    for (std::string& it : sources) { \
      it += stringFormat("(%d)", \
              dispatcher()->get<HANDLE>(it)->dispatcher()->values().size()); \
    } \
    if (sources.size() == 0) { \
      ss << SHORTNAME << " "; \
    } else { \
      *dst << "\n  * Channel " << SHORTNAME << ": " << join(sources, ", "); \
    } \
  }
  FOREACH_CHANNEL(DISP_CHANNEL)
#undef DISP_CHANNEL

  *dst << "\nNavDataset internal dispatcher: ";
  *dst << _dispatcher;

  *dst << "\n\n  * The following channels are not part of this dataset: " << ss.str() << "\n" << std::endl;

  for (auto it : _activeSource) {
    *dst << "Active source for " << descriptionForCode(it.first)
      << ": " << it.second->source() << std::endl;
  }
}

bool NavDataset::isDefaultConstructed() const {
  return !_dispatcher;
}

std::ostream &operator<<(std::ostream &s, const NavDataset &ds) {
  ds.outputSummary(&s);
  return s;
}

NavDataset NavDataset::clone() const {
  if (_dispatcher == nullptr) {
    return NavDataset();
  }

  NavDataset r(
      cloneAndfilterDispatcher(
          _dispatcher.get(),
          [](DataCode testedCode, const std::string&) {
            return true;
          }),
      _lowerBound,
      _upperBound);
  r._activeSource = _activeSource;
  return r;
}

NavDataset NavDataset::stripChannel(DataCode code) const {
  if (_dispatcher == nullptr) {
    return NavDataset();
  }

  NavDataset r(
      cloneAndfilterDispatcher(
          _dispatcher.get(),
          [code](DataCode testedCode, const std::string&) {
            return testedCode != code;
          }),
      _lowerBound,
      _upperBound);
  r._activeSource = _activeSource;
  r.clearSourceSelection(code);
  return r;
}

std::shared_ptr<DispatchData> NavDataset::activeChannelOrNull(DataCode code) const {
  if (!_dispatcher) {
    return std::shared_ptr<DispatchData>();
  }

  auto it = _activeSource.find(code);
  if (it != _activeSource.end()) {
    return it->second;
  }

  auto codeMapIt = _dispatcher->allSources().find(code);
  if (codeMapIt ==  _dispatcher->allSources().end()
      || codeMapIt->second.size() != 1) {
    return std::shared_ptr<DispatchData>();
  }

  return codeMapIt->second.begin()->second;
}

std::shared_ptr<DispatchData> NavDataset::activeChannel(DataCode code) const {
  std::shared_ptr<DispatchData> r = activeChannelOrNull(code);
  if (!r) {
    auto sources = _dispatcher->sourcesForChannel(code);
    if (sources.size() == 0) {
      return std::shared_ptr<DispatchData>();
    } else {
      LOG(FATAL) << "there are multiple sources for channel "
        << descriptionForCode(code) << ", but no active source is selected. "
        << "Sources: " << join(sources, ", ");
    }
  }
  return r;
}

void NavDataset::selectSource(DataCode code, const std::string& source) {
  CHECK(_dispatcher) << "can't select a source for an empty NavDataset";

  std::shared_ptr<DispatchData> ptr
    = _dispatcher->dispatchDataForSource(code, source);

  CHECK(ptr) << "No source " << source << " in channel "
    << descriptionForCode(code) << ". Valid sources: [ "
    << join(_dispatcher->sourcesForChannel(code), ", ") << " ]";

  _activeSource[code] = ptr;
}

void NavDataset::preferSource(std::set<DataCode> codes,
                              const std::string& source) {
  for(DataCode code : codes) {
    preferSource(code, source);
  }
}

bool NavDataset::preferSource(DataCode code, const std::string& source) {
  if (hasSource(code, source)) {
    selectSource(code, source);
    return true;
  } else {
    return false;
  }
}

// Activate "source" on all channels for which it is valid
void NavDataset::preferSourceAll(const std::string& source) {
#define PREFER_CHANNEL(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  preferSource(HANDLE, source);
  FOREACH_CHANNEL(PREFER_CHANNEL)
#undef PREFER_CHANNEL
}

namespace {

  bool selected(DataCode code, const std::set<DataCode>& selection) {
    return selection.size() == 0 || selection.find(code) != selection.end();
  }

  template <class T>
  class PublishListener : public Listener<T> {
   public:
    PublishListener<T>(Dispatcher *d, DataCode code, Duration<> interval)
      : Listener<T>(interval), _dispatcher(d), _code(code) { }
    virtual void onNewValue(const ValueDispatcher<T> &valueDispatcher) {
      _values.push_back(TimedValue<T>(valueDispatcher.lastTimeStamp(),
                                       valueDispatcher.lastValue()));
      sources.insert(_dispatcher->dispatchData(_code)->source());
    }

    NavDataset addChannel(const NavDataset& ds) {
      std::string source;
      if (sources.size() == 1) {
        /* Before, it used to be "source = *sources.begin();"
         *
         * PublishListener is used for createMergedChannels.
         * Every DataCode that should be merged has its own
         * PublishListener. The purpose of the PublishListener
         * is to merge the data from other channels, into _values,
         * see the onNewValue method. The addChannel method is
         * called at the end of the merging process to build the
         * new dataset. The new dataset is built starting from
         * the old dataset. If sources.size() is 1, it means that
         * there is only one source being merged, and that is
         * pointless. That source is already present in ds, that
         * we pass to this method, so there is no point in
         * putting back the values. But before this change,
         * that's what we would do. And by doing that, we would
         * start dropping old values due to size limitations of
         * the DispatchData where we put the values.
         *
         * We can't directly return ds because ds might have multiple sources,
         * but the other sources were occluded by a higher priority main
         * source. See #1201. Instead, we make sure the only source we have
         * is the one selected in the result.
         */
        NavDataset result = ds.clone();
        result.selectSource(_code, *sources.begin());
        return result;
      } else {
        source = "mix (";
        source += join(sources, ", ") + ")";
      }

      if (_values.size() > 0) {
        NavDataset result = ds.addChannel<T>(_code, source, _values);
        result.selectSource(_code, source);
        return result;
      } else {
        return ds;
      }
    }

    virtual ~PublishListener() {}
   private:
    Dispatcher *_dispatcher;
    typename TimedSampleCollection<T>::TimedVector _values;
    DataCode _code;
    std::set<std::string> sources;
  };


} // namespace

NavDataset NavDataset::createMergedChannels(std::set<DataCode> channelSelection,
                                            Duration<> minInterval) {
  if (!_dispatcher) {
    return NavDataset();
  }

  NavDataset result{clone()};

  ReplayDispatcher replay;

#define LISTEN_TO(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  PublishListener<TYPE> HANDLE##Listener(&replay, HANDLE, minInterval); \
  if (selected(HANDLE, channelSelection)) { \
    replay.get<HANDLE>()->dispatcher()->subscribe(& HANDLE##Listener); \
  }
FOREACH_CHANNEL(LISTEN_TO)
#undef LISTEN_TO

  replay.replay(_dispatcher.get());

#define ADD_CHANNEL(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  if (selected(HANDLE, channelSelection)) { \
    result = HANDLE##Listener . addChannel(result); \
  }
FOREACH_CHANNEL(ADD_CHANNEL)
#undef ADD_CHANNEL

  return result;
}

NavDataset NavDataset::preferSourceOrCreateMergedChannels(
    std::set<DataCode> channelSelection,
    const std::string& source) const {
  NavDataset result = *this;
  result.preferSource(channelSelection, source);
  std::set<DataCode> needMerge;
  for(DataCode code : channelSelection) {
    if (!result.hasActiveChannel(code)) {
      needMerge.insert(code);
    }
  }
  if (needMerge.size() == 0) {
    return result;
  } else {
    return result.createMergedChannels(needMerge);
  }
}


}  // namespace sail

