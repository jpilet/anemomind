/*
 * SegmentOutlierFilter.h
 *
 *  Created on: 4 Aug 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_SEGMENTOUTLIERFILTER_H_
#define SERVER_MATH_SEGMENTOUTLIERFILTER_H_

#include <server/common/indexed.h>
#include <server/math/QuadSegment.h>
#include <set>
#include <vector>
#include <server/common/Transducer.h>
#include <random>

namespace sail {
namespace sof {

template <int N>
using Vec = std::array<double, N>;

template <int N>
using Pair = std::pair<double, Vec<N>>;


struct Settings {
  double reg2 = 1.0;
  double reg1 = 1.0e-6;
  double maxCost = 100.0;
  bool verbosity = 1; // 0, 1, 2
};

template <int N>
Array<QuadSegment<N>> initializeSegments(
    const Array<Pair<N>>& points) {
  int n = points.size();
  Array<QuadSegment<N>> dst(n);
  for (int i = 0; i < n; i++) {
    const auto& x = points[i];
    dst[i] = QuadSegment<N>::primitive(i, x.first, x.second);
  }
  return dst;
}

template <int N>
Array<QuadSegment<N>> concatenatePairs(
    const Array<QuadSegment<N>>& segments,
    const Settings& settings) {
  std::default_random_engine rng;
  auto arr = segments;
  int n = arr.size();
  int outCount = div1(n, 2);
  int inputIndex = 0;

  typedef std::pair<double, QuadSegment<N>> WithCost;

  auto T = composeTransducers(
      Map<WithCost, int>([&](int k) {
        if (k == 2) {
          auto a = segments[inputIndex + 0];
          auto b = segments[inputIndex + 1];
          auto c = join(a, b, settings.reg2, settings.reg1);
          return WithCost{c.cost() - a.cost() - b.cost(), c};
        } else {
          return WithCost{0, segments[inputIndex]};
        }
        inputIndex += k;
      }),
      Filter<WithCost>(std::function<bool(WithCost)>(
          [&](const WithCost& q) {
        if (q.first < settings.maxCost) {
          return true;
        } else {
          if (1 <= settings.verbosity) {
            LOG(INFO) << "Dropping segment " << q.second.leftMost()
                << ".." << q.second.rightMost();
          }
          return false;
        }
      })),
      Map<QuadSegment<N>, WithCost>([](const WithCost& x) {
        return x.second;
      }));
  std::uniform_int_distribution<int> distrib(0, n-1);
  int index = distrib(rng);
  auto iter = arr.begin();
  auto step = T.apply(iteratorStep(iter));
  bool odd = n % 2 == 1;
  for (int i = 0; i < outCount; i++) {
    iter = step.step(iter, odd && i == index? 1 : 2);
  }
  int resultSize = std::distance(arr.begin(), iter);
  return segments.sliceTo(resultSize);
}

template <int N>
Array<bool> optimize(
    const Array<Pair<N>>& points,
    const Settings& settings) {
  Array<QuadSegment<N>> segments = initializeSegments<N>(points);
  auto mask = Array<bool>::fill(segments.size(), true);
  while (1 < segments.size()) {
    if (1 <= settings.verbosity) {
      LOG(INFO) << "Sweep over " << segments.size() << " segments";
    }
    segments = concatenatePairs(segments, settings);
  }
  if (1 <= settings.verbosity) {
    LOG(INFO) << "Final segment count: " << segments.size();
  }
  return mask;
}

}
} /* namespace sail */

#endif /* SERVER_MATH_SEGMENTOUTLIERFILTER_H_ */
