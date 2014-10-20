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

  Array<Spani> temporalSplit(Array<Nav> sortedNavs, Duration<double> thresh, int offset = 0) {
    int gapCount = countGaps(sortedNavs, thresh);
    int from = 0;
    int count = sortedNavs.size() - 1;
    int splitCount = gapCount + 1;
    ArrayBuilder<Spani> dst(splitCount);
    for (int i = 0; i < count; i++) {
      Duration<double> dif = sortedNavs[i+1].time() - sortedNavs[i].time();
      if (dif >= thresh) {
        int to = i+1;
        dst.add(Spani(offset + from, offset + to));
        from = to;
      }
    }
    dst.add(Spani(offset + from, offset + sortedNavs.size()));
    assert(dst.size() == splitCount);
    return dst.get();
  }
}


Array<Spani> temporalSplit(Array<Nav> sortedNavs,
    double relativeThresh, Duration<double> lowerThresh, int offset) {
    Duration<double> dif = sortedNavs.last().time() - sortedNavs.first().time();
    Duration<double> rel = relativeThresh*dif;
    Duration<double> thresh = std::max(rel, lowerThresh);
    return temporalSplit(sortedNavs, thresh, offset);
}

namespace {
  void recursiveTemporalSplitSub(Array<Nav> sortedNavs,
      double relativeThresh, Duration<double> lowerThresh,
      ArrayBuilder<Spani> *dst, int offset) {
    Array<Spani> splitted = temporalSplit(sortedNavs, relativeThresh, lowerThresh, 0);
    if (splitted.size() == 1) {
      Spani x = splitted[0];
      dst->add(Spani(x.minv() + offset, x.maxv() + offset));
    } else {
      for (auto x: splitted) {
        recursiveTemporalSplitSub(sortedNavs.slice(x.minv(), x.maxv()),
            relativeThresh, lowerThresh, dst, offset + x.minv());
      }
    }
  }
}

Array<Spani> recursiveTemporalSplit(Array<Nav> sortedNavs,
    double relativeThresh, Duration<double> lowerThresh) {
    ArrayBuilder<Spani> dst;
    recursiveTemporalSplitSub(sortedNavs, relativeThresh, lowerThresh, &dst, 0);
    return dst.get();
}



}
