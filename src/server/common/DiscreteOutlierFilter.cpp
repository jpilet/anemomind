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
    std::cout << "Segment is " << seg.minv() << " to " << seg.maxv() << std::endl;
    std::cout << "  it should be " << segmentMask[i] << std::endl;
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
    std::cout << "Iteration " << i << std::endl;
    BackPointer ptr;
    ptr.cost = std::numeric_limits<double>::max();
    for (int j = 0; j < settings.backSteps.size(); j++) {
      int steps = settings.backSteps[j];
      assert(1 <= steps);
      int prev = std::max(i - steps, -1);
      BackPointer cand;
      cand.previous = prev;
      std::cout << "  Consider " << prev << std::endl;
      int skipped = std::max(0, steps - 1);
      cand.cost = skipped*settings.threshold;
      std::cout << "  Skipped cost: " << cand.cost << std::endl;
      if (0 <= prev && i < n) {
        cand.previous = prev;
        auto backCost = backPointers[prev].cost;
        auto transCost = evalTransitionCost(
            costFun, spans[prev], spans[i]);
        cand.cost += backCost + transCost;
      }
      std::cout << "    The cost is " << cand.cost << std::endl;
      std::cout << "    The index is " << cand.previous << std::endl;
      ptr = std::min(ptr, cand);
      std::cout << "  The updated cost is " << ptr.cost << std::endl;
      std::cout << "  The updated index is " << ptr.previous << std::endl;
    }
    backPointers[i] = ptr;
    std::cout << "  The final cost is " << ptr.cost << std::endl;
    std::cout << "  The final index is " << ptr.previous << std::endl;
  }
  std::cout << "Done" << std::endl;
  for (int i = 0; i <= n; i++) {
    auto ptr = backPointers[i];
    std::cout << "  Backpointer " << i
        << ", prev=" << ptr.previous << " cost="
        << ptr.cost << std::endl;
  }
  auto segmentMask = Array<bool>::fill(n, false);
  int i = n;
  while (true) {
    std::cout << "Visiting " << i << std::endl;
    i = backPointers[i].previous;
    std::cout << " go to " << i << std::endl;
    if (0 <= i) {
      std::cout << "Set the segment mask" << std::endl;
      segmentMask[i] = true;
    } else {
      break;
    }
  }
  std::cout << "Build the mask" << std::endl;
  return buildSampleMask(segmentMask, spans);
}

Array<bool> computeOutlierMaskFromPairwiseCosts(
    int n, std::function<double(int, int)> cost,
    const Settings &settings) {
  int pairCount = n-1;
  if (pairCount <= 0) {
    return Array<bool>::fill(n, true);
  }

  std::cout << "Find the gaps" << std::endl;
  ArrayBuilder<int> splits(pairCount+2);
  splits.add(0);
  ArrayBuilder<double> splitCosts(pairCount);
  for (int i = 0; i < pairCount; i++) {
    if (settings.threshold < cost(i, i+1)) {
      int index = i + 1;
      splits.add(index);
      splitCosts.add(index);
    }
  }
  std::cout << "Add the last point" << std::endl;
  splits.add(n);
  auto spans = makeSpans(splits.get());
  std::cout << "Let's solve it!"<< std::endl;
  return solveMask(spans, cost, settings);
}

}
}
