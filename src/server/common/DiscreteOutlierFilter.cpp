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

namespace sail {
namespace DiscreteOutlierFilter {

Array<Span<int>> makeSpans(const Array<int> &pts) {
  int n = pts.size() - 1;
  Array<Span<int>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = Span<int>(pts[i], pts[i+1]);
  }
  return dst;
}

struct BackPointer {
  int previous = -1;
  double cost = 0.0;

  bool operator<(const BackPointer &other) const {
    return cost < other.cost;
  }
};

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

Array<bool> solveMask(
    const Array<Span<int>> &spans,
    std::function<double(int, int)> costFun,
    const Settings &settings) {
  int n = spans.size();
  auto backPointers = Array<BackPointer>::fill(n+1, BackPointer());

  for (int i = 1; i <= n; i++) {
    BackPointer ptr;
    ptr.cost = std::numeric_limits<double>::max();
    for (int j = 0; j < settings.backSteps.size(); j++) {
      int steps = settings.backSteps[j];
      assert(1 <= steps);
      int prev = std::max(i - steps, -1);
      BackPointer cand;
      cand.previous = prev;
      int skipped = std::max(0, steps - 1);
      cand.cost = skipped*settings.threshold;
      if (0 <= prev && i < n) {
        cand.previous = prev;
        auto backCost = backPointers[prev].cost;
        auto transCost = evalTransitionCost(
            costFun, spans[prev], spans[i]);
        cand.cost += backCost + transCost;
      }
      ptr = std::min(ptr, cand);
    }
    backPointers[i] = ptr;
  }

  auto segmentMask = Array<bool>::fill(n, false);
  int i = n;
  while (true) {
    i = backPointers[i].previous;
    if (0 <= i) {
      segmentMask[i] = true;
    } else {
      break;
    }
  }
  return buildSampleMask(segmentMask, spans);
}

Array<bool> computeOutlierMaskFromPairwiseCosts(
    int n, std::function<double(int, int)> cost,
    std::function<bool(int, int)> cut,
    const Settings &settings) {
  int pairCount = n-1;
  if (pairCount <= 0) {
    return Array<bool>::fill(n, true);
  }

  ArrayBuilder<int> splits(pairCount+2);
  splits.add(0);
  ArrayBuilder<double> splitCosts(pairCount);
  for (int i = 0; i < pairCount; i++) {
    if (settings.threshold < cost(i, i+1) || cut(i, i+1)) {
      int index = i + 1;
      splits.add(index);
      splitCosts.add(index);
    }
  }
  splits.add(n);
  auto spans = makeSpans(splits.get());
  return solveMask(spans, cost, settings);
}

}
}
