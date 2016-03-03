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

#include <memory>
#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/TimeStamp.h>
#include <device/anemobox/TimedSampleCollection.h>


namespace sail {

// In order to view a slice
// of a TimedSampleCollection<T>::TimedVector
template <typename T>
class TimedSampleRange {
 public:
  typedef typename sail::TimedSampleCollection<T>::TimedVector TimedVector;
  typedef typename TimedVector::const_iterator Iterator;
  typedef TimedSampleRange<T> ThisType;

  TimedSampleRange() : _defined(false) {}

  TimedSampleRange(const Iterator &b, const Iterator &e) :
    _defined(b <= e), _begin(b), _end(e) {}


  Iterator begin() const {return _begin;}
  Iterator end() const {return _end;}

  int size() const {return _defined? _end - _begin : 0;}

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

  const TimedValue<T> &operator[] (int i) const {
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
  bool _defined;

  Iterator _begin, _end;
};

const std::shared_ptr<DispatchData> &getMergedDispatchData(
  DataCode code,
  const std::shared_ptr<std::map<DataCode, std::shared_ptr<DispatchData>>> &merged,
  const std::shared_ptr<Dispatcher> &dispatcher);

class Dispatcher;
class NavDataset {
public:
  NavDataset() {}
  NavDataset(const std::shared_ptr<Dispatcher> &dispatcher,
      const std::shared_ptr<std::map<DataCode, std::shared_ptr<DispatchData> > > &merged
        = std::make_shared<std::map<DataCode, std::shared_ptr<DispatchData> > >(),
      TimeStamp a = TimeStamp(), TimeStamp b = TimeStamp());

  NavDataset slice(TimeStamp a, TimeStamp b) const;
  NavDataset sliceFrom(TimeStamp ts) const;
  NavDataset sliceTo(TimeStamp ts) const;
  NavDataset sliceFirst(const Duration<double> &dur) const;
  NavDataset sliceLast(const Duration<double> &dur) const;

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
    auto m = getMergedSamples<Code>();
    if (m == nullptr) {
      return TimedSampleRange<typename TypeForCode<Code>::type>();
    }
    const auto &v = m->samples();
    auto lower = (_lowerBound.defined()? std::lower_bound(v.begin(), v.end(), _lowerBound) : v.begin());
    auto upper = (_upperBound.defined()? std::upper_bound(v.begin(), v.end(), _upperBound) : v.end());
    return TimedSampleRange<typename TypeForCode<Code>::type>(lower, upper);
  }

  template <DataCode Code>
  Optional<typename TypeForCode<Code>::type> lookUpPrioritizedSample(
      TimeStamp time, const Array<std::string> &orderedSources) const {
    typedef typename TypeForCode<Code>::type T;
    const auto &all = _dispatcher->allSources();
    auto found = all.find(Code);
    if (found != all.end()) {
      const auto &sources = found->second;
      for (auto srcName: orderedSources) {
        auto found = sources.find(srcName);
        if (found != sources.end()) {
          const TimedSampleCollection<T> &data = toTypedDispatchData<Code>(found->second.get())->dispatcher()->values();
          auto nearest = findNearestTimedValue<T>(data.samples().begin(), data.samples().end(), time);
          if (nearest.defined()) {
            if (std::abs((nearest.get().time - time).seconds()) < maxMergeDifSeconds) {
              return Optional<T>(nearest.get().value);
            }
          }
        }
      }
    }
    return Optional<T>();
  }

  void outputSummary(std::ostream *dst) const;

  bool operator== (const NavDataset &other) const {
    return _dispatcher == other._dispatcher && _lowerBound == other._lowerBound
        && _upperBound == other._upperBound;
  }
private:
  template <DataCode Code>
  const TimedSampleCollection<typename TypeForCode<Code>::type> *getMergedSamples() const {
    if (!_merged) { // Only when we have a default-constructed, completely empty object.
      assert(!_dispatcher);
      return nullptr;
    }
    const auto &d = getMergedDispatchData(Code, _merged, _dispatcher);
    if (d) {
      auto dp = d.get();
      return &(toTypedDispatchData<Code>(dp)->dispatcher()->values());
    }
    return nullptr;
  }

  // Undefined _lowerBound means negative infinity,
  // Undefined _upperBound means positive infinity.
  sail::TimeStamp _lowerBound, _upperBound;
  std::shared_ptr<Dispatcher> _dispatcher;

  std::shared_ptr<std::map<DataCode, std::shared_ptr<DispatchData>>> _merged;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_NAVDATASET_H_ */
