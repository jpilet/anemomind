/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/Dispatcher.h>
#include <server/nautical/NavDataset.h>
#include <assert.h>
#include <device/anemobox/DispatcherUtils.h>

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

NavDataset::NavDataset(const std::shared_ptr<Dispatcher> &dispatcher, TimeStamp a, TimeStamp b) :
  _dispatcher(dispatcher), _lowerBound(a), _upperBound(b) {
  assert(beforeOrEqual(_lowerBound, _upperBound, true));
}

NavDataset NavDataset::slice(TimeStamp a, TimeStamp b) const {
  assert(beforeOrEqual(a, b, true));
  assert(beforeOrEqual(_lowerBound, a, true));
  assert(beforeOrEqual(b, _upperBound, true));
  return NavDataset(_dispatcher, a, b);
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
  visitDispatcherChannels(_dispatcher, &visitor);
  return NavDataset(_dispatcher, visitor.lowerBound(), visitor.upperBound());
}

void NavDataset::outputSummary(std::ostream *dst) const {
  std::stringstream ss;
  *dst << "\n\nNavDataset summary:";
#define DISP_CHANNEL(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  { \
    auto x = getTheChannel<HANDLE>(); if (x.empty()) { \
      ss << #HANDLE << " "; \
    } else { \
      *dst << "\n  * Channel " << #HANDLE << " (" << DESCRIPTION << ") with " << x.size() << " values"; \
    } \
  }
  FOREACH_CHANNEL(DISP_CHANNEL)
#undef DISP_CHANNEL

  *dst << "\n\n  * The following channels are not part of this dataset: " << ss.str() << "\n" << std::endl;
}


}
