/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "SpanOverlap.h"
#include <algorithm>

namespace sail {

namespace {
  class EndPoint {
   public:
    EndPoint() : _spanIndex(-1), _position(-1), _risingEdge(false) {}
    EndPoint(int spanIndex_, int position_, bool risingEdge_) :
      _spanIndex(spanIndex_), _position(position_), _risingEdge(risingEdge_) {}
    bool operator<(const EndPoint &other) const {
      return _position < other._position;
    }

    bool risingEdge() const {return _risingEdge;}
    int spanIndex() const {return _spanIndex;}
    int position() const {return _position;}
   private:
    int _spanIndex, _position;
    bool _risingEdge;
  };

  Array<EndPoint> listSortedEndPts(Array<Spani> spans) {
    int spanCount = spans.size();
    int endptCount = 2*spanCount;
    Array<EndPoint> endpts(endptCount);
    for (int spanIndex = 0; spanIndex < spanCount; spanIndex++) {
      int at = 2*spanIndex;
      Spani span = spans[spanIndex];
      endpts[at + 0] = EndPoint(spanIndex, span.minv(), true);
      endpts[at + 1] = EndPoint(spanIndex, span.maxv(), false);
    }
    std::sort(endpts.begin(), endpts.end());
    return endpts;
  }
}

Array<SpanOverlap> SpanOverlap::compute(Array<Spani> spans) {
  Array<EndPoint> endpts = listSortedEndPts(spans);
  if (endpts.empty()) {
    return Array<SpanOverlap>();
  }
  int spanCount = spans.size();
  Arrayi allIndices = makeRange(spanCount);
  Arrayb activeSpans = Arrayb::fill(spanCount, false);
  int currentPos = endpts.first().position();
  int overlapCounter = 0;
  Array<SpanOverlap> overlaps(endpts.size());
  for (auto ep : endpts) {
    int newPos = ep.position();
    if (newPos != currentPos) {
      overlaps[overlapCounter] = SpanOverlap(Spani(currentPos, newPos), allIndices.slice(activeSpans));
      overlapCounter++;
      currentPos = newPos;
    }
    activeSpans[ep.spanIndex()] = ep.risingEdge();
  }
  assert(currentPos == endpts.last().position());
  return overlaps.sliceTo(overlapCounter);
}

} /* namespace sail */
