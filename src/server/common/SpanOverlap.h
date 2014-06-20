/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SPANOVERLAP_H_
#define SPANOVERLAP_H_

#include <server/common/Span.h>
#include <server/common/Array.h>
#include <algorithm>
#include <server/common/ArrayBuilder.h>

namespace sail {
namespace SpanOverlapImplementation {

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

Array<EndPoint> listSortedEndPts(Array<Spani> spans);

}



template <typename T>
class SpanOverlap {
 public:
  typedef SpanOverlap<T> ThisType;

  SpanOverlap() {}
  SpanOverlap(Spani span_, Array<T> objects_) : _span(span_), _objects(objects_) {}
  Spani span() const {return _span;}
  Array<T> objects() const {return _objects;}

  bool operator== (const ThisType &other) const {
    return _span.minv() == other._span.minv() && _span.maxv() == other._span.maxv() && _objects == other._objects;
  }


  static Array<ThisType> compute(Array<Spani> spans, Array<T> objects) {
    assert(spans.size() == objects.size());
    using namespace SpanOverlapImplementation;
    Array<EndPoint> endpts = listSortedEndPts(spans);
    if (endpts.empty()) {
      return Array<ThisType>();
    }
    Arrayb activeSpans = Arrayb::fill(spans.size(), false);
    int currentPos = endpts.first().position();
    ArrayBuilder<ThisType> overlaps(endpts.size());
    for (auto ep : endpts) {
      int newPos = ep.position();
      if (newPos != currentPos) {
        overlaps.add(ThisType(Spani(currentPos, newPos), objects.slice(activeSpans)));
        currentPos = newPos;
      }
      activeSpans[ep.spanIndex()] = ep.risingEdge();
    }
    assert(currentPos == endpts.last().position());
    return overlaps.get();
  }

 private:
  // The span of this overlap
  Spani _span;

  // Indices to the original spans that overlap here.
  Array<T> _objects;
};

typedef SpanOverlap<int> SpanOverlapi;


} /* namespace sail */

#endif /* SPANOVERLAP_H_ */
