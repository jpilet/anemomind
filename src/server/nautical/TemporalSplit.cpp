/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TemporalSplit.h>
#include <cassert>
#include <server/common/logging.h>
#include <server/common/Histogram.h>
#include <server/common/ProportionateIndexer.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <server/common/PhysicalQuantityIO.h>

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



/*class SplitNode {
 public:
  SplitNode(Array<Nav> navs, Spani span)
};*/

struct Dif {
 Duration<double> dur;
 int index;
};

std::ostream &operator<<(std::ostream &s, const Dif &x) {
  s << "Dif of duration " << x.dur.seconds() << " seconds" << std::endl;
  return s;
}

Array<Dif> getDifs(Array<Nav> navs) {
  std::sort(navs.begin(), navs.end());
  int n = navs.size() - 1;
  Array<Dif> difs(n);
  for (int i = 0; i < n; i++) {
    difs[i] = Dif{navs[i+1].time() - navs[i].time(), i+1};
  }

  class Cmp {
   public:
    bool operator() (const Dif &a, const Dif &b) const {
      return a.dur > b.dur;
    }
  };

  std::sort(difs.begin(), difs.end(), Cmp());
  return difs;
}

class SplitNode {
 public:
  SplitNode(Array<Nav> navs) : _navs(navs), _span(0, navs.size()) {}

  SplitNode(Array<Nav> navs, Spani span) : _navs(navs), _span(span) {}

  bool isLeaf() const {
    return !bool(_left);
  }

  void split(int index) {
    if (isLeaf()) {
      _left = std::shared_ptr<SplitNode>(new SplitNode(_navs, Spani(_span.minv(), index)));
      _right = std::shared_ptr<SplitNode>(new SplitNode(_navs, Spani(index, _span.maxv())));
    } else {
      if (index < _left->span().maxv()) {
        _left->split(index);
      } else {
        _right->split(index);
      }
    }
  }

  Spani span() const {
    return _span;
  }

  void disp(std::ostream *dst, Duration<double> minGap, int depth = 0) {
    indent(dst, 2*depth);
    *dst << "[begin node of duration " << duration().seconds() << " seconds]\n";
    if (!isLeaf()) {
      auto gapDur = gapDuration();
      if (gapDur > minGap) {
        int nextDepth = depth+1;
        _left->disp(dst, minGap, nextDepth);
        indent(dst, 2*nextDepth);
        *dst << "Gap duration " << gapDur.seconds() << " seconds" << std::endl;
        _right->disp(dst, minGap, depth+1);
      }
    }
    indent(dst, 2*depth);
    *dst << "[end node of duration " << duration().seconds() << " seconds]\n";
  }

  const Nav &first() const {
    return _navs[_span.minv()];
  }

  const Nav &last() const {
    return _navs[_span.maxv()-1];
  }

  Duration<double> duration() const {
    return last().time() - first().time();
  }

  int middle() const {
    return _left->span().maxv();
  }

  Duration<double> gapDuration() const {
    return _right->first().time() - _left->last().time();
  }

  Duration<double> meanPeriodTime() const {
    return (1.0/(_span.width() - 1))*duration();
  }

  bool isRoot() const {
    return _span.minv() == 0 && _span.maxv() == _navs.size();
  }

  Array<Duration<double> > getDescendingPeriodTimes(
      ArrayBuilder<Duration<double> > *temp = nullptr) const {
    if (isRoot() && temp == nullptr) {
      ArrayBuilder<Duration<double> > durs;
      getDescendingPeriodTimes(&durs);
      auto result = durs.get();
      std::sort(result.begin(), result.end(), std::greater<Duration<double> >());
      return result;
    } else {
      if (!isLeaf()) {
        temp->add(meanPeriodTime());
        _left->getDescendingPeriodTimes(temp);
        _right->getDescendingPeriodTimes(temp);
      }
      return Array<Duration<double> >();
    }
  }

 private:
  Spani _span;
  Array<Nav> _navs;
  std::shared_ptr<SplitNode> _left, _right;
};

std::shared_ptr<SplitNode> makeSplitTree(Array<Nav> navs) {
  auto difs = getDifs(navs);
  std::shared_ptr<SplitNode> result(new SplitNode(navs));
  for (auto dif: difs) {
    result->split(dif.index);
  }
  return result;
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
  std::cout << EXPR_AND_VAL_AS_STRING(getDifs(navs).sliceTo(180)) << std::endl;

  auto tree = makeSplitTree(navs);
  //tree->disp(&std::cout, Duration<double>::seconds(12));
  //std::cout << EXPR_AND_VAL_AS_STRING(tree->getDescendingPeriodTimes().sliceTo(60)) << std::endl;

}





}
