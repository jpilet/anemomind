/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TemporalSplit.h>
#include <cassert>
#include <server/common/logging.h>

namespace sail {

using namespace NavCompat;

namespace {
  int countGaps(const NavDataset &sortedNavs,
      Duration<double> thresh) {
    int count = getNavSize(sortedNavs) - 1;
    int counter = 0;
    for (int i = 0; i < count; i++) {
      Duration<double> dif = getNav(sortedNavs, i+1).time() - getNav(sortedNavs, i).time();
      assert(dif.seconds() >= 0);
      if (dif >= thresh) {
        counter++;
      }
    }
    return counter;
  }

  Array<Spani> temporalSplit(NavDataset sortedNavs, Duration<double> thresh, int offset = 0) {
    int n = getNavSize(sortedNavs);
    CHECK_LT(0, n);
    int gapCount = countGaps(sortedNavs, thresh);
    int from = 0;
    int count = n - 1;
    int splitCount = gapCount + 1;
    ArrayBuilder<Spani> dst(splitCount);
    for (int i = 0; i < count; i++) {
      Duration<double> dif = getNav(sortedNavs, i+1).time() - getNav(sortedNavs, i).time();
      if (dif >= thresh) {
        int to = i+1;
        dst.add(Spani(offset + from, offset + to));
        from = to;
      }
    }
    dst.add(Spani(offset + from, offset + n));
    assert(dst.size() == splitCount);
    return dst.get();
  }
}


Array<Spani> temporalSplit(NavDataset sortedNavs,
    double relativeThresh, Duration<double> lowerThresh, int offset) {
    Duration<double> dif = getLast(sortedNavs).time() - getFirst(sortedNavs).time();
    Duration<double> rel = relativeThresh*dif;
    Duration<double> thresh = std::max(rel, lowerThresh);
    return temporalSplit(sortedNavs, thresh, offset);
}

namespace {
  void recursiveTemporalSplitSub(NavDataset sortedNavs,
      double relativeThresh, Duration<double> lowerThresh,
      ArrayBuilder<Spani> *dst, int offset) {
    Array<Spani> splitted = temporalSplit(sortedNavs, relativeThresh, lowerThresh, 0);
    if (splitted.size() == 1) {
      Spani x = splitted[0];
      dst->add(Spani(x.minv() + offset, x.maxv() + offset));
    } else {
      for (auto x: splitted) {
        recursiveTemporalSplitSub(slice(sortedNavs, x.minv(), x.maxv()),
            relativeThresh, lowerThresh, dst, offset + x.minv());
      }
    }
  }
}

Array<Spani> recursiveTemporalSplit(NavDataset sortedNavs,
    double relativeThresh, Duration<double> lowerThresh) {
    ArrayBuilder<Spani> dst;
    recursiveTemporalSplitSub(sortedNavs, relativeThresh, lowerThresh, &dst, 0);
    return dst.get();
}

void dispTemporalRaceOverview(Array<Spani> spans, NavDataset navs, std::ostream *out) {
  int spanCount = spans.size();
  for (int i = 0; i < spans.size(); i++) {
    Spani span = spans[i];
    NavDataset sub = slice(navs, span.minv(), span.maxv());
    *out << "[" << span.minv() << ", " << span.maxv() << "[" << std::endl;
    *out << "   from     " << getFirst(sub).time().toString() << std::endl;
    *out << "   duration " << (getLast(sub).time() - getFirst(sub).time()).str() << std::endl;
    *out << "   to     " << getLast(sub).time().toString() << '\n' << std::endl;
    if (i < spans.size()-1) {
      Duration<double> gap = getNav(navs, spans[i+1].minv()).time() - getLast(sub).time();
      *out << "Gap of " << gap.str() << '\n' << std::endl;
    }
  }
  *out << "Total of " << spanCount << " race episodes." << std::endl;
}



}
