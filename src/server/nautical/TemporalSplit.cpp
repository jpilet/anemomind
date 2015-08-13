/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TemporalSplit.h>
#include <cassert>
#include <server/common/logging.h>
#include <server/common/Histogram.h>

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
    CHECK_LT(0, sortedNavs.size());
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

Array<Duration<double> > getDifs(Array<Nav> navs) {
  int n = navs.size() - 1;
  Array<Duration<double> > difs(n);
  for (int i = 0; i < n; i++) {
    difs[i] = navs[i+1].time() - navs[i].time();
  }
  return difs;
}



void dispHist(Array<Nav> navs, int binCount) {
  auto minDur = Duration<double>::seconds(0.001);
  auto maxDur = Duration<double>::days(3);
  auto durs = getDifs(navs);
  HistogramMap<double, false> m(binCount, log(minDur.seconds()), log(maxDur.seconds()));
  auto logDurs = durs.map<double>([&](Duration<double> x) {return log(x.seconds());});
  Arrayi countPerBin = m.countPerBin(logDurs);
  for (int i = 0; i < m.binCount(); i++) {
    std::cout << "From " << exp(m.toLeftBound(i)) << " seconds to " << exp(m.toRightBound(i)) << " seconds: " << countPerBin[i] << std::endl;
  }
}

void dispTemporalRaceOverview(Array<Spani> spans, Array<Nav> navs, std::ostream *out) {
  int spanCount = spans.size();
  for (int i = 0; i < spans.size(); i++) {
    Spani span = spans[i];
    Array<Nav> sub = navs.slice(span.minv(), span.maxv());
    *out << "[" << span.minv() << ", " << span.maxv() << "[" << std::endl;
    *out << "   from          " << sub.first().time().toString() << std::endl;
    *out << "   duration      " << (sub.last().time() - sub.first().time()).str() << std::endl;
    *out << "   to            " << sub.last().time().toString() << std::endl;
    Duration<double> periodTime = (1.0/sub.size())*(sub.last().time() - sub.first().time());
    *out << "   period time   " << (periodTime).seconds() << " seconds\n" << std::endl;
    if (i < spans.size()-1) {
      Duration<double> gap = navs[spans[i+1].minv()].time() - sub.last().time();
      *out << "Gap of " << gap.str() << '\n' << std::endl;
    }
  }
  *out << "Total of " << spanCount << " race episodes." << std::endl;
  dispHist(navs, 30);
}



}
