/*
 * DiscreteOutlierFilter.cpp
 *
 *  Created on: 4 Nov 2016
 *      Author: jonas
 */

#include <server/common/DiscreteOutlierFilter.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Span.h>
#include <iostream>
#include <server/common/ArrayIO.h>
#include <server/nautical/GeographicPosition.h>

namespace sail {
namespace DiscreteOutlierFilter {

Duration<double> addDurs(
    const Duration<double> &a,
    const Duration<double> &b) {
  return a + b;
}

Array<Duration<double>> getDurs(const Array<Span<TimeStamp>> &spans) {
  int n = spans.size();
  int last = n-1;
  Array<Duration<double>> durs(n);
  for (int i = 0; i < last; i++) {
    durs[i] = spans[i+1].minv() - spans[i].minv();
  }
  durs[last] = spans[last].maxv() - spans[last].minv();
  return durs;
}

TimeSpanIndexer::TimeSpanIndexer(
    const Array<Span<TimeStamp>> &timeSpans) :
        _tree(addDurs, getDurs(timeSpans)),
        _spans(timeSpans) {
  if (!timeSpans.empty()) {
    _offset = timeSpans.first().minv();
  }
}


int TimeSpanIndexer::lookUp(TimeStamp t) const {
  if (!_offset.defined()) {
    return -1;
  }
  return _tree.findLeafIndex(t - _offset);
}

TimeStamp TimeSpanIndexer::getTime(int i) const {
  return i == -1? _offset : _spans[i].maxv();
}


std::ostream &operator<<(std::ostream &s,
    const BackPointer &bptr) {
  s << "BackPointer(prev=" << bptr.previous << ", cost=" << bptr.cost << ")";
  return s;
}

Array<Span<int>> makeSpans(const Array<int> &pts) {
  int n = pts.size() - 1;
  Array<Span<int>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = Span<int>(pts[i], pts[i+1]);
  }
  return dst;
}

double evalTransitionCost(
    std::function<double(int, int)> f,
    Span<int> a, Span<int> b) {
  int i = a.maxv()-1;
  int j = b.minv();
  assert(i < j);
  return f(i, j);
}

Array<bool> buildSampleMask(const Array<bool> &segmentMask,
    const Array<Span<int>> &spans) {
  assert(0 < segmentMask.size());
  assert(segmentMask.size() == spans.size());
  int n = spans.last().maxv();
  auto dst = Array<bool>::fill(n, false);
  for (int i = 0; i < segmentMask.size(); i++) {
    auto seg = spans[i];
    if (segmentMask[i]) {
      for (auto j: seg) {
        dst[j] = true;
      }
    }
  }
  return dst;
}

Array<Array<BackPointer>> buildBackPointers(
    const Array<Span<int>> &spans,
        std::function<double(int, int)> costFun,
        Array<int> backSteps,
        double threshold) {
  int n = spans.size();
  auto data = Array<BackPointer>(n*backSteps.size());
  Array<Array<BackPointer>> dst(n);
  for (int i = 1; i < n+1; i++) {
    auto sub = data.sliceBlock(i-1, backSteps.size());
    dst[i-1] = sub;
    for (int j = 0; j < backSteps.size(); j++) {
      BackPointer ptr;
      ptr.previous = std::max(-1, i - backSteps[j]);
      int stepSize = std::max(0, i - ptr.previous - 1);
      ptr.cost = stepSize*threshold;
      if (0 <= ptr.previous && i < n) {
        ptr.cost += evalTransitionCost(
            costFun, spans[ptr.previous], spans[i]);
      }
      sub[j] = ptr;
    }
  }

  return dst;
}

Array<bool> unwindBackPointers(const Array<BackPointer> &path) {
  int n = path.size()-1;
  auto segmentMask = Array<bool>::fill(n, false);
  int i = n;
  while (true) {
    i = path[i].previous;
    if (0 <= i) {
      segmentMask[i] = true;
    } else {
      break;
    }
  }
  return segmentMask;
}

Array<bool> solveBackPointers(
    const Array<Array<BackPointer>> &pointers) {
  int n = pointers.size();
  Array<BackPointer> path(n+1);
  for (int i_ = 0; i_ < n; i_++) {
    int i = i_+1;
    auto ptrs = pointers[i_];
    BackPointer best;
    best.cost = std::numeric_limits<double>::max();
    for (int j = 0; j < ptrs.size(); j++) {
      BackPointer candidate;
      candidate = ptrs[j];
      if (0 <= candidate.previous) {
        candidate.cost += path[candidate.previous].cost;
      }
      best = std::min(best, candidate);
    }
    path[i] = best;
  }
  return unwindBackPointers(path);
}


Array<bool> solveMask(const Array<Span<int>> spans,
    const std::function<double(int,int)> costFun,
    const Settings &settings) {
  auto pointers = buildBackPointers(
      spans, costFun, settings.backSteps,
      settings.threshold);
  auto mask = solveBackPointers(pointers);
  return buildSampleMask(mask, spans);
}

Array<int> computeSplits(int n,
    std::function<bool(int, int)> cut) {
  int pairCount = n-1;
  if (pairCount <= 0) {
    return Array<int>{0, n};
  }
  ArrayBuilder<int> splits(pairCount+2);
  splits.add(0);
  ArrayBuilder<double> splitCosts(pairCount);
  for (int i = 0; i < pairCount; i++) {
    if (cut(i, i+1)) {
      int index = i + 1;
      splits.add(index);
      splitCosts.add(index);
    }
  }
  splits.add(n);
  return splits.get();
}

Array<Span<int>> segment(int n,
    std::function<bool(int, int)> cut) {
  return makeSpans(computeSplits(n, cut));
}

typedef decltype(1.0_units/1.0_s) CostPerTime;

template <typename T>
Array<Span<TimeStamp>> toTimeSpans(
    const Array<Span<int>> &segments,
    const Array<TimedValue<T>> &data) {
  int n = segments.size();
  Array<Span<TimeStamp>> dst(n);
  for (int i = 0; i < n; i++) {
    auto x = segments[i];
    dst[i] = Span<TimeStamp>(data[x.minv()].time, data[x.maxv()-1].time);
  }
  return dst;
}

template <typename T>
Array<Array<BackPointer>> buildTemporalBackPointers(
    const Array<Span<int>> &segments,
    const Array<TimedValue<T>> &data,
    double threshold,
    const Array<Duration<double>> &backSteps,
    std::function<double(TimedValue<T>, TimedValue<T>)> costFun,
    CostPerTime costPerTime) {
  auto timeSpans = toTimeSpans(segments, data);
  std::cout << "Allocate the indexer" << std::endl;
  TimeSpanIndexer indexer(timeSpans);
  int n = segments.size();
  Array<BackPointer> allPointers(n*backSteps.size());
  Array<Array<BackPointer>> dst(n);
  std::cout << "Looping over it all " << std::endl;
  for (int i = 1; i < n+1; i++) {
    std::cout << "Allocate for " << i << std::endl;
    int i0 = i-1;
    auto sub = allPointers.sliceBlock(i0, backSteps.size());
    dst[i0] = sub;
    auto currentTime = i < n?
        timeSpans[i].minv() : timeSpans.last().maxv();
    std::cout << "Consider back steps" << std::endl;
    for (int j = 0; j < backSteps.size(); j++) {
      std::cout << "Consider back step " << j << std::endl;
      BackPointer ptr;
      ptr.previous = indexer.lookUp(currentTime - backSteps[j]);
      auto dur = currentTime - indexer.getTime(ptr.previous);
      ptr.cost = double(dur*costPerTime);
      if (0 <= ptr.previous && i < n) {
        ptr.cost += costFun(
            data[segments[ptr.previous].maxv()-1],
            data[segments[i].minv()]);
      }
      sub[j] = ptr;
      std::cout << "Assigned it for " << j << std::endl;
    }
  }
  return dst;
}

template <typename T>
Array<bool> identifyOutliers(
    const Array<TimedValue<T>> &data,
    std::function<double(const TimedValue<T>&,
        const TimedValue<T> &)> cost,
    Array<Duration<double>> backSteps,
    double threshold) {
  if (data.size() < 2) {
    return Array<bool>::fill(data.size(), true);
  }

  std::cout << "Build segments..." << std::endl;
  auto segments = segment(data.size(), [=](int i, int j) {
    return threshold < cost(data[i], data[j]);
  });

  std::cout << "Built " << segments.size() << std::endl;
  auto dur = data.last().time - data.first().time;
  CostPerTime costPerTime = (threshold*segments.size()*1.0_units)/dur;
  std::cout << "Build pointers"<< std::endl;
  auto ptrs = buildTemporalBackPointers<T>(
      segments, data, threshold, backSteps,
      cost, costPerTime);
  std::cout << "Built pointers" << std::endl;
  return buildSampleMask(solveBackPointers(ptrs), segments);
}

#define INSTANTIATE_IDENTIFY_OUTLIERS(T) \
template Array<bool> identifyOutliers<T>( \
    const Array<TimedValue<T>> &data, \
    std::function<double(const TimedValue<T>&, \
        const TimedValue<T> &)> cost, \
    Array<Duration<double>> backSteps, \
    double threshold);

INSTANTIATE_IDENTIFY_OUTLIERS(double)
INSTANTIATE_IDENTIFY_OUTLIERS(HorizontalMotion<double>)
INSTANTIATE_IDENTIFY_OUTLIERS(GeographicPosition<double>)

Array<bool> computeOutlierMaskFromPairwiseCosts(
    int n, std::function<double(int, int)> cost,
    std::function<bool(int, int)> cut,
    const Settings &settings) {
  auto spans = segment(n, [=](int i, int j) {
    return cut(i, j) || settings.threshold < cost(i, j);
  });
  return solveMask(spans, cost, settings);
}

}
}
