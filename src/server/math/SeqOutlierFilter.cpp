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
  std::vector<int> successors, predecessors;

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
  x->successors.push_back(y->arrayIndex);
  y->predecessors.push_back(x->arrayIndex);
}

void connectToSuccessors(Array<SegmentInfo>* data, int index) {
  auto& x = (*data)[index];
  int minExtent = std::numeric_limits<int>::max();
  for (int i = index+1; i < data->size(); i++) {
    auto& y = (*data)[i];

    // Any further connections would never be needed, because it
    // would always be more optimal to include the segment that
    // resulted in the current value of 'minExtent' and then connect
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
    dst.insert(i);
  }
  return dst;
}

std::set<int> computeInlierSegments(
    const Array<IndexSpan>& spans) {
  auto total = computeTotalSpans(spans);
  buildGraph(&total);
  int best = -1;
  for (auto& x: total) {
    x.computeBestPredecessor(total);
    best = bestSegment(best, x.arrayIndex, total);
  }
  return getInlierSegmentSet(total, best);
}

}
}
