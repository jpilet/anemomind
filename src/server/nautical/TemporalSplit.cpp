/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TemporalSplit.h>
#include <cassert>

namespace sail {

namespace {
  int countGaps(const Array<Nav> &sortedNavs,
      Duration<double> thresh) {
    int count = sortedNavs.size() - 1;
    int counter = 0;
    for (int i = 0; i < count; i++) {
      Duration<double> dif = sortedNavs[i+1].time() - sortedNavs[i].time();
      assert(dif.seconds() >= 0);
      if (dif >= thresh) {
        counter++;
      }
    }
    return counter;
  }

  Array<Spani> temporalSplit(Array<Nav> sortedNavs, Duration<double> thresh) {
    int gapCount = countGaps(sortedNavs, thresh);
    int from = 0;
    int count = sortedNavs.size() - 1;
    int splitCount = gapCount + 1;
    ArrayBuilder<Spani> dst(splitCount);
    for (int i = 0; i < count; i++) {
      Duration<double> dif = sortedNavs[i+1].time() - sortedNavs[i].time();
      if (dif >= thresh) {
        int to = i+1;
        dst.add(Spani(from, to));
        from = to;
      }
    }
    dst.add(Spani(from, sortedNavs.size()));
    assert(dst.size() == splitCount);
    return dst.get();
  }
}


Array<Spani> temporalSplit(Array<Nav> sortedNavs,
    double relativeThresh, Duration<double> lowerThresh) {
    Duration<double> dif = sortedNavs.last().time() - sortedNavs.first().time();
    Duration<double> rel = relativeThresh*dif;
    Duration<double> thresh = std::max(rel, lowerThresh);
    return temporalSplit(sortedNavs, thresh);
}

namespace {
  void recursiveTemporalSplitSub(Array<Nav> sortedNavs,
      double relativeThresh, Duration<double> lowerThresh,
      ArrayBuilder<Spani> *dst) {
    Array<Spani> splitted = temporalSplit(sortedNavs, relativeThresh, lowerThresh);
    if (splitted.size() == 1) {
      dst->add(splitted[0]);
    } else {
      for (auto x: splitted) {
        recursiveTemporalSplitSub(sortedNavs.slice(x.minv(), x.maxv()),
            relativeThresh, lowerThresh, dst);
      }
    }
  }
}

Array<Spani> recursiveTemporalSplit(Array<Nav> sortedNavs,
    double relativeThresh, Duration<double> lowerThresh) {
    ArrayBuilder<Spani> dst;
    recursiveTemporalSplitSub(sortedNavs, relativeThresh, lowerThresh, &dst);
    return dst.get();
}



}
