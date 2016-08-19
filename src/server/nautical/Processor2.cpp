/*
 * Processor2.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include "Processor2.h"
#include <server/common/ArrayBuilder.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/PathBuilder.h>
#include <server/common/PhysicalQuantityIO.h>
#include <server/common/ArrayIO.h>
#include <fstream>
#include <server/common/ReduceTree.h>
#include <server/common/logging.h>

namespace sail {
namespace Processor2 {

class GpsTimesVisitor {
 public:
  ArrayBuilder<TimeStamp> times;

  template <DataCode Code, typename T>
  void visit(const char *shortName, const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {
      if (Code == GPS_POS) {
        for (auto x: coll) {
          times.add(x.time);
        }
      }
  }
};


Settings::Settings() {
  logRoot = "/tmp/";

  subSessionCut = Duration<double>::minutes(3.0);
  mainSessionCut = Duration<double>::hours(1.0);
  minCalibDur = Duration<double>::hours(3.0);

  sessionCutSettings.cuttingThreshold = Duration<double>::hours(1.0);
  sessionCutSettings.regularization = 1.0;
}

std::string Settings::makeLogFilename(const std::string &s) {
  return PathBuilder::makeDirectory(logRoot)
    .makeFile(s).get().toString();
}

Array<TimeStamp> getAllGpsTimeStamps(const Dispatcher *d) {
  GpsTimesVisitor v;
  visitDispatcherChannelsConst(d, &v);
  auto times =  v.times.get();
  std::sort(times.begin(), times.end());
  return times;
}

void addUnique(std::vector<int> *dst, int i) {
  if (dst->empty() or dst->back() != i) {
    dst->push_back(i);
  }
}

Array<Span<TimeStamp> > segmentSubSessions(
    const Array<TimeStamp> &times,
    const Settings &settings) {
  if (times.empty()) {
    return Array<Span<TimeStamp> >();
  }

  std::vector<int> cuts{0};
  int n = times.size() - 1;
  for (int i = 0; i < n; i++) {
    auto dur = times[i+1] - times[i];
    if (settings.subSessionCut < dur) {
      cuts.push_back(i+1);
    }
  }
  addUnique(&cuts, times.size());
  ArrayBuilder<Span<TimeStamp> > spans;
  int m = cuts.size() - 1;
  for (int i = 0; i < m; i++) {
    int from = cuts[i];
    int to = cuts[i+1]-1;
    spans.add(Span<TimeStamp>(times[from], times[to]));
  }
  return spans.get();
}

void outputTimeSpansToFile(
    const std::string &filename,
    const Array<Span<TimeStamp> > &timeSpans) {
  std::ofstream file(filename);
  for (int i = 0; i < timeSpans.size(); i++) {
    auto s = timeSpans[i];
    file << "Span " << i+1 << " of "
        << timeSpans.size() << ": " << s.minv()
        << " to " << s.maxv() << std::endl;
  }
}

void outputGroupsToFile(
      const std::string &filename,
      const Array<Spani> &groups,
      const Array<Span<TimeStamp> > sessions) {
  std::ofstream file(filename);
  for (int i = 0; i < groups.size(); i++) {
    file << "Group " << i+1 << " of " << groups.size() << std::endl;
    auto g = groups[i];
    for (auto j: g) {
      auto span = sessions[j];
      file << "   Span from " << span.minv() << " to " << span.maxv()
          << std::endl;
    }
  }
}

namespace {
  Duration<double> dur(const Span<TimeStamp> &span) {
    return span.maxv() - span.minv();
  }

  struct SpanCand {
    int index;
    Duration<double> d;

    bool operator<(const SpanCand &other) const {
      return d < other.d;
    }
  };

  int findIndex(const ReduceTree<Spani> &tree,
      int indexToSearchFor, int nodeIndex) {
    if (tree.isLeaf(nodeIndex)) {
      return nodeIndex;
    } else {
      int leftIndex = tree.left(nodeIndex);
      Spani leftSpan = tree.getNodeValue(leftIndex);
      if (indexToSearchFor < leftSpan.minv()) {
        return -1;
      } else if (leftSpan.contains(indexToSearchFor)) {
        return findIndex(tree, indexToSearchFor, leftIndex);
      } else {
        int rightIndex = tree.right(nodeIndex);
        if (!tree.contains(rightIndex)) {
          return -1;
        } else {
          Spani rightSpan = tree.getNodeValue(rightIndex);
          if (rightSpan.contains(indexToSearchFor)) {
            return findIndex(tree, indexToSearchFor, rightIndex);
          } else {
            return -1;
          }
        }
      }
    }
  }


  Spani merge(Spani a, Spani b) {
    Spani c = a;
    c.extend(b);
    return c;
  }

  void mergeSpans(ReduceTree<Spani> *tree, int l, int r) {
    auto a = tree->getNodeValue(l);
    auto b = tree->getNodeValue(r);
    tree->setNodeValue(l, merge(a, b));
    tree->setNodeValue(r, Spani(b.maxv(), b.maxv()));
  }

  bool empty(Spani x) {
    return x.width() == 0;
  }
}

Array<Spani> groupSessionsByThreshold(
    const Array<Span<TimeStamp> > &timeSpans,
    const Duration<double> &threshold) {
  auto n = timeSpans.size();
  ArrayBuilder<Spani> dst(n);
  int from = 0;
  for (int i = 0; i < n-1; i++) {
    auto a = timeSpans[i];
    auto b = timeSpans[i+1];
    if (b.minv() - a.maxv() > threshold) {
      int to = i+1;
      dst.add(Spani(from, to));
      from = to;
    }
  }
  dst.add(Spani(from, n));
  return dst.get();
}

Array<Spani> computeCalibrationGroups(
  Array<Span<TimeStamp> > timeSpans,
  Duration<double> minCalibDur) {
  int n = timeSpans.size();
  std::cout << "Number of time spans: " << n << std::endl;
  Array<Duration<double> > cumulative(n+1);
  cumulative[0] = Duration<double>::seconds(0.0);
  Array<Spani> spans(n);
  for (int i = 0; i < n; i++) {
    cumulative[i+1] = cumulative[i] + dur(timeSpans[i]);
    spans[i] = Spani(i, i+1);
  }

  std::cout << "Built cumulative" << std::endl;

  auto dur = [&](Spani x) {
    return cumulative[x.maxv()] - cumulative[x.minv()];
  };

  ReduceTree<Spani> indexTree(&merge, spans);

  ReduceTree<Spani> durationTree(
      [&](Spani a, Spani b) {
    if (empty(a)) {
      return b;
    } else if (empty(b)) {
      return a;
    } else {
      return dur(a) < dur(b)? a : b;
    }
  }, spans);
  std::cout << "Built reduce tree" << std::endl;

  for (int i = 0; i < n-1; i++) {
    std::cout << "Iteration " << i << std::endl;
    std::cout << "Leaves: " << indexTree.leaves() << std::endl;
    auto shortest = durationTree.top();
    std::cout << "  Shortest: " << shortest << std::endl;
    auto shortestDur = dur(shortest);
    std::cout << "  Duration: " << shortestDur << std::endl;
    int shortestIndex = findIndex(indexTree, shortest.minv(), 0);
    if (minCalibDur <= shortestDur) {
      std::cout << "  long enought, we are done." << std::endl;
      break;
    }
    auto considerPair = [&](int current, int cand) {
      int candSpanIndex = findIndex(indexTree, cand, 0);
      if (candSpanIndex == -1) {
        return SpanCand{-1, Duration<double>::seconds(
            std::numeric_limits<double>::infinity())};
      } else {
        return SpanCand{candSpanIndex,
          timeSpans[std::max(current, cand)].minv() -
          timeSpans[std::min(current, cand)].maxv()};
      }
    };
    auto best = std::min(
        considerPair(shortest.minv(), shortest.minv()-1),
        considerPair(shortest.maxv()-1, shortest.maxv()));
    if (best.index == -1) {
      LOG(FATAL) << "This should not happen";
      break;
    }
    int l = std::min(shortestIndex, best.index);
    int r = std::max(shortestIndex, best.index);
    mergeSpans(&indexTree, l, r);
    mergeSpans(&durationTree, l, r);
  }
  auto leaves = indexTree.leaves();
  ArrayBuilder<Spani> nonEmpty;
  for (auto leaf: leaves) {
    if (!empty(leaf)) {
      nonEmpty.add(leaf);
    }
  }
  return nonEmpty.get();
}



}
} /* namespace sail */
