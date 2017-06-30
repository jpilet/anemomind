/*
 * SeqOutlierFilter.cpp
 *
 *  Created on: 15 Jun 2017
 *      Author: jonas
 */
#include <server/math/SeqOutlierFilter.h>
#include <map>

namespace sail {
namespace SeqOutlierFilter {


void IndexGrouper::flush() {
  if(_counter - _lastFrom > 0) {
    IndexSpan x;
    x.index = _lastIndex;
    x.span = Span<int>(_lastFrom, _counter);
    _groups.add(x);
  }
}

void IndexGrouper::insert(int index) {
  if (index != _lastIndex) {
    flush();
    _lastFrom = _counter;
  }
  _lastIndex = index;
  _counter++;
}

Array<IndexSpan> IndexGrouper::get() {
  flush();
  return _groups.get();
}

struct SegmentInfo;

int bestSegment(int a, int b, const Array<SegmentInfo>& data);

struct SegmentInfo {
  int arrayIndex = -1;
  Span<int> totalExtent;
  int totalSamples = 0;
  int segmentIndex = -1;
  int bestPredecessor = -1;
  int cumulativeSampleCount = 0;
  std::vector<int> predecessors;

  void computeBestPredecessor(const Array<SegmentInfo>& data) {
    for (auto i: predecessors) {
      bestPredecessor = bestSegment(bestPredecessor, i, data);
    }
    cumulativeSampleCount = totalSamples +
        (bestPredecessor == -1? 0
            : data[bestPredecessor].cumulativeSampleCount);
  }
};

int bestSegment(int a, int b, const Array<SegmentInfo>& data) {
  if (a == -1) {
    return b;
  } else if (b == -1) {
    return a;
  }
  return data[a].cumulativeSampleCount < data[b].cumulativeSampleCount?
      b : a;
}


Array<SegmentInfo> computeTotalSpans(
    const Array<IndexSpan>& spans) {
  std::map<int, SegmentInfo> dst;
  int counter = 0;
  for (auto span: spans) {
    auto& x = dst[span.index];
    x.segmentIndex = span.index;
    x.totalExtent.extend(span.span);
    x.totalSamples += span.span.width();
    x.arrayIndex = x.arrayIndex == -1? counter++ : x.arrayIndex;
  }
  Array<SegmentInfo> flat(counter);
  for (auto kv: dst) {
    flat[kv.second.arrayIndex] = kv.second;
  }
  return flat;
}

void connect(SegmentInfo* x, SegmentInfo* y) {
  y->predecessors.push_back(x->arrayIndex);
}

void connectToSuccessors(Array<SegmentInfo>* data, int index) {
  auto& x = (*data)[index];
  int minExtent = std::numeric_limits<int>::max();
  for (int i = index+1; i < data->size(); i++) {
    auto& y = (*data)[i];

    // In case the condition below holds, any further
    // connections would never be needed, because it
    // would always be more optimal to include the segment that
    // resulted in the current value of 'minExtent' and connect
    // that segment to 'y', than omitting the segment corresponding to
    // 'minExtent'.
    if (minExtent <= y.totalExtent.minv()) {
      break;
    }
    // Test if non-overlapping.
    if (x.totalExtent.maxv() <= y.totalExtent.minv()) {
      connect(&x, &y);
      minExtent = std::min(minExtent, y.totalExtent.maxv());
    }
  }
}

void buildGraph(Array<SegmentInfo>* data) {
  for (int i = 0; i < data->size(); i++) {
    connectToSuccessors(data, i);
  }
}

std::set<int> getInlierSegmentSet(
    const Array<SegmentInfo>& data, int final) {
  std::set<int> dst;
  for (int i = final; i != -1; i = data[i].bestPredecessor) {
    dst.insert(data[i].segmentIndex);
  }
  return dst;
}

std::set<int> computeInlierSegments(
    const Array<IndexSpan>& spans) {

  // Because we can have multiple contiguous
  // spans with the same segment index, we group them here
  // into an array of SegmentInfo. The array index is
  // *does not* correspond to the segment index, but is
  // used as a reference in the dynamic programming problem that
  // we will solve.
  auto total = computeTotalSpans(spans);

  // Here we connect every segment with its closest successors
  // that are not overlapping with it.
  buildGraph(&total);

  // Here we do a forward sweep in the dynamic programming problem.
  // The objective is to choose a subset of the SegmentInfo elements
  // such that the total number of samples is maximized, subject to
  // the constraint that 'totalSpan' of two SegmentInfo cannot overlap.
  // That constraint is encoded in the graph that we built previously.
  int best = -1;
  for (auto& x: total) {
    x.computeBestPredecessor(total);
    best = bestSegment(best, x.arrayIndex, total);
  }

  // Here we unwind the solution starting from the best final segment.
  return getInlierSegmentSet(total, best);
}

}
}
