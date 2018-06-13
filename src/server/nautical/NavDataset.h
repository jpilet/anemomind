/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 *
 *  Represents a dataset of measurements from the sensors
 *  onboard. Implemented using a dispatcher, but the dispatcher
 *  should be invisible to the user of NavDataset.
 *
 *  The user of NavDataset should not need to worry about
 *  choosing the right channel to read from, etc.
 */

#ifndef SERVER_NAUTICAL_NAVDATASET_H_
#define SERVER_NAUTICAL_NAVDATASET_H_

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/DispatcherUtils.h>
#include <device/anemobox/TimedSampleCollection.h>
#include <memory>
#include <server/common/TimeStamp.h>
#include <server/common/logging.h>
#include <server/nautical/types/SampledSignal.h>


namespace sail {

// In order to view a slice
// of a TimedSampleCollection<T>::TimedVector
template <typename T>
class TimedSampleRange : public SampledSignal<T> {
 public:
  typedef typename sail::TimedSampleCollection<T>::TimedVector TimedVector;
  typedef typename TimedVector::const_iterator Iterator;
  typedef TimedSampleRange<T> ThisType;

  static const TimedVector& emptyVector() { static TimedVector e; return e; }

  TimedSampleRange() :
      _defined(false),
      _begin(emptyVector().begin()),
      _end(emptyVector().end()) {}

  TimedSampleRange(const Iterator &b, const Iterator &e) :
    _defined(b <= e), _begin(b), _end(e) {}


  Iterator begin() const {return _begin;}
  Iterator end() const {return _end;}

  size_t size() const override {return _defined? _end - _begin : 0;}

  bool empty() const {return (_defined? _begin == _end : true);}

  const TimedValue<T> &first() const {
    assert(!empty());
    return *_begin;
  }

  const TimedValue<T> &last() const {
    assert(!empty());
    return *(_end - 1);
  }

  ThisType slice(int from, int to) const {
    if (_defined) {
      assert(0 <= from);
      assert(from <= to);
      assert(to <= size());
      return ThisType(_begin + from, _begin + to);
    } else {
      assert(from == 0);
      assert(to == 0);
      return ThisType();
    }
  }

  TimedValue<T> operator[] (int i) const override {
    assert(0 <= i && i < size());
    return *(_begin + i);
  }

  Optional<TimedValue<T> > nearest(TimeStamp t) const {
    if (empty()) {
      return Optional<TimedValue<T> >();
    }
    return findNearestTimedValue<T, Iterator>(_begin, _end, t);
  }

  bool operator== (const ThisType &other) const {
    if (_defined) {
      return _begin == other._begin && _end == other._end && other._defined;
    }
    return !other._defined;
  }
 private:
  // Seems like iterators don't have any meaningful
  // default constructors (http://stackoverflow.com/a/3395263).
  // So in order to distinguish a TimedSampleRange that is
  // default constructed, from one that is not, use a _defined flag.

/* Julien's response:
It seems to me that there is no need for a default constructor. If we need
something not initialized, we should use an Optional<>. However, our current
implementation of Optional does not work with classes that can't be default
constructed. We should use boost::optional, which supports it. They implement
it with a tricky hack:

https://github.com/boostorg/optional/blob/develop/include/boost/optional/detail/optional_aligned_storage.hpp#L35

Let's keep that for another PR. Please add a TODO to replace Optional
by boost::optional.
 */

  bool _defined;

  Iterator _begin, _end;
};

const std::shared_ptr<DispatchData> &getMergedDispatchData(
  DataCode code,
  const std::shared_ptr<std::map<DataCode, std::shared_ptr<DispatchData>>> &merged,
  const std::shared_ptr<Dispatcher> &dispatcher);

class Dispatcher;


/*
 * The NavDataset class is essentially a reference to a dispatcher.
 * A Dispatcher is essentially "append only": it is possible to append samples,
 * but not to alter them. NavDataset provides a way to have a view on a dispatcher,
 * potentially modified, or restricted to some particular period.
 *
 * No methods modify the object itself. Instead, methods return a new NavDataset
 * containing references to existing data. Duplication is done only when necessary.
 * For example, the following line produces a new NavDataset that points to the same
 * data, except that there will be no GPS_POS channel:
 * NavDataset stripped = navDataset.stripChannel<GPS_POS>();
 *
 * In a NavDataset, there can be multiple sources for the same channel. But
 * reading can occur only from one channel. There are two options:
 *   - selecting an "active source"
 *   - or having only one source (in that case the source becomes "active" by default)
 *
 * Note that it is also possible to merge multiple sources together into a
 * merged source, add this source to the NavDataset, and set it active.
 * In that case, reading this channel would result in reading a mix of multiple
 * sources.
 */
class NavDataset {
public:
  NavDataset() {}

  NavDataset(const std::shared_ptr<Dispatcher> &dispatcher,
      TimeStamp a = TimeStamp(), TimeStamp b = TimeStamp(),
      std::map<DataCode, std::shared_ptr<DispatchData>> activeSource
        = std::map<DataCode, std::shared_ptr<DispatchData>>());

  NavDataset slice(TimeStamp a, TimeStamp b) const;
  NavDataset sliceFrom(TimeStamp ts) const;
  NavDataset sliceTo(TimeStamp ts) const;
  NavDataset sliceFirst(const Duration<double> &dur) const;
  NavDataset sliceLast(const Duration<double> &dur) const;

  NavDataset overrideChannels(
        const std::map<DataCode, std::map<std::string,
          std::shared_ptr<DispatchData>>> &toAdd) const;

  NavDataset overrideChannels(
      const std::string &srcName,
        const std::map<DataCode,
        std::shared_ptr<DispatchData>> &toAdd) const;

  template<typename T>
  NavDataset replaceChannel(
      DataCode code,
      const std::string& source,
      const typename TimedSampleCollection<T>::TimedVector& values) const {
    NavDataset result = stripChannel(code);
    result.dispatcher()->insertValues<T>(code, source, values);
    return result;
  }

  template<typename T>
  NavDataset addChannel(
      DataCode code,
      const std::string& source,
      const typename TimedSampleCollection<T>::TimedVector& values) const {
    NavDataset r;
    if (_dispatcher) {
      // we can't modify dispatcher directly, because it is shared.
      // We have to clone it first.
      r = clone();
    } else {
      r._dispatcher =  std::make_shared<Dispatcher>();
    }
    r.dispatcher()->insertValues<T>(code, source, values);
    return r;
  }

  template<typename T>
  NavDataset addAndSelectChannel(
      DataCode code,
      const std::string& source,
      const typename TimedSampleCollection<T>::TimedVector& values) const {
    NavDataset r(addChannel<T>(code, source, values));
    r.selectSource(code, source);
    return r;
  }

  // Returns a new NavDataset in which the channels contained in
  // channelSelection are merged with the priority source selection algorithm
  // used by the dispatcher during navigation.
  // If channelSelection is empty, all channels are merged.
  // minInterval will downsample data.
  NavDataset createMergedChannels(
      std::set<DataCode> channelSelection = std::set<DataCode>(),
      Duration<> minInterval = Duration<>::seconds(0));

  // Returns a new NavDataset with a cloned dispatcher that is guaranteed not
  // to be shared with anybody else, meaning it can be modified.
  NavDataset clone() const;

  NavDataset stripChannel(DataCode code) const;
  NavDataset stripSource(const std::string& source) const;

  bool hasLowerBound() const;
  bool hasUpperBound() const;
  bool isBounded() const;
  TimeStamp lowerBound() const;
  TimeStamp upperBound() const;
  Duration<double> duration() const;

  NavDataset fitBounds() const;

  // Return a range of samples given the code.
  template <DataCode Code>
  TimedSampleRange<typename TypeForCode<Code>::type> samples() const {
    if (!_dispatcher) {
      return TimedSampleRange<typename TypeForCode<Code>::type>();
    }
    std::shared_ptr<DispatchData> ptr = activeChannel(Code);
    if (!ptr) {
      return TimedSampleRange<typename TypeForCode<Code>::type>();
    }

    const typename TimedSampleCollection<typename TypeForCode<Code>::type>::TimedVector&
      v = toTypedDispatchData<Code>(ptr.get())->dispatcher()->values().samples();

    auto lower = (_lowerBound.defined()? std::lower_bound(v.begin(), v.end(), _lowerBound) : v.begin());
    auto upper = (_upperBound.defined()? std::upper_bound(v.begin(), v.end(), _upperBound) : v.end());
    return TimedSampleRange<typename TypeForCode<Code>::type>(lower, upper);
  }

  // returns a one line string describing bounds.
  std::string boundsAsString() const;
  void outputSummary(std::ostream *dst) const;

  bool operator== (const NavDataset &other) const {
    return _dispatcher == other._dispatcher && _lowerBound == other._lowerBound
        && _upperBound == other._upperBound;
  }

  // TODO: Use this method sparingly: Preferably use samples() whenever
  // possible.
  const std::shared_ptr<Dispatcher> &dispatcher() const {
    return _dispatcher;
  }

  // Can used to check whether some processing step failed. That processing
  // step will then return 'NavDataset()', for which this method returns true.
  bool isDefaultConstructed() const;

  // Returns a pointer to the active DispatchData for <code>.
  // If there is only one source for the given channel, it will be
  // automatically used as active.
  // If there are multiple sources, one of sourcesForChannel(code)
  // has to be selected with selectSource().
  //
  // If there is channel ambiguity, the OrNull version returns a null pointer
  // while activeChannel() crashes with a relevant error message.
  std::shared_ptr<DispatchData> activeChannel(DataCode code) const;
  std::shared_ptr<DispatchData> activeChannelOrNull(DataCode code) const;

  bool hasActiveChannel(DataCode code) const {
    return static_cast<bool>(activeChannelOrNull(code));
  }

  std::vector<std::string> sourcesForChannel(DataCode code) const {
    if (!_dispatcher) {
      return std::vector<std::string>();
    } else {
      return _dispatcher->sourcesForChannel(code);
    }
  }

  bool hasSource(DataCode code, const std::string& source) const {
    return _dispatcher && _dispatcher->hasSource(code, source);
  }

  // select the active source for a given channel.
  // Will crash if source is not a valid one for this channel.
  void selectSource(DataCode code, const std::string& source);

  // if the source if valid for "code", set it as active and return true.
  // Otherwise, return false.
  bool preferSource(DataCode code, const std::string& source);
  void preferSource(std::set<DataCode> codes, const std::string& source);


  // Activate "source" on all channels for which it is valid
  void preferSourceAll(const std::string& source);

  void clearSourceSelection(DataCode code) {
    _activeSource.erase(code);
  }

  NavDataset preferSourceOrCreateMergedChannels(
      std::set<DataCode> channelSelection,
      const std::string& source) const;
private:

  // Undefined _lowerBound means negative infinity,
  // Undefined _upperBound means positive infinity.
  sail::TimeStamp _lowerBound, _upperBound;
  std::shared_ptr<Dispatcher> _dispatcher;

  std::map<DataCode, std::shared_ptr<DispatchData>> _activeSource;

  
};

std::ostream &operator<<(std::ostream &s, const NavDataset &ds);

} // namespace sail

#endif /* SERVER_NAUTICAL_NAVDATASET_H_ */
