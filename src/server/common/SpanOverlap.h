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

template <typename T>
class EndPoint {
 public:
  typedef EndPoint<T> ThisType;
  EndPoint() : _spanIndex(-1), _position(-1), _risingEdge(false) {}
  EndPoint(int spanIndex_, T position_, bool risingEdge_) :
    _spanIndex(spanIndex_), _position(position_), _risingEdge(risingEdge_) {}
  bool operator<(const ThisType &other) const {
    return _position < other._position;
  }

  bool risingEdge() const {return _risingEdge;}
  int spanIndex() const {return _spanIndex;}
  T position() const {return _position;}
 private:
  int _spanIndex;
  T _position;
  bool _risingEdge;
};

template <typename T>
Array<EndPoint<T> > listSortedEndPts(Array<Span<T> > spans) {
    int endptCount = 2*spans.size();
    Array<EndPoint<T> > endpts(endptCount);
    for (int spanIndex = 0; spanIndex < spans.size(); spanIndex++) {
      int at = 2*spanIndex;
      Span<T> span = spans[spanIndex];
      endpts[at + 0] = EndPoint<T>(spanIndex, span.minv(), true);
      endpts[at + 1] = EndPoint<T>(spanIndex, span.maxv(), false);
    }
    std::sort(endpts.begin(), endpts.end());
    return endpts;
  }

}



template <typename T, typename EPType=int>
class SpanOverlap {
 private:
  typedef Span<EPType> Sp;
 public:
  typedef SpanOverlap<T, EPType> ThisType;

  SpanOverlap() {}
  SpanOverlap(Sp span_, Array<T> objects_) : _span(span_), _objects(objects_) {}
  Sp span() const {return _span;}
  Array<T> objects() const {return _objects;}

  bool operator== (const ThisType &other) const {
    return _span.minv() == other._span.minv() && _span.maxv() == other._span.maxv() && _objects == other._objects;
  }


  static Array<ThisType> compute(Array<Sp> spans, Array<T> objects) {
    typedef SpanOverlapImplementation::EndPoint<EPType> EP;
    assert(spans.size() == objects.size());
    using namespace SpanOverlapImplementation;
    Array<EP> endpts = listSortedEndPts(spans);
    if (endpts.empty()) {
      return Array<ThisType>();
    }
    Arrayb activeSpans = Arrayb::fill(spans.size(), false);
    EPType currentPos = endpts.first().position();
    ArrayBuilder<ThisType> overlaps(endpts.size());
    for (auto ep : endpts) {
      EPType newPos = ep.position();
      if (newPos != currentPos) {
        overlaps.add(ThisType(Sp(currentPos, newPos), objects.slice(activeSpans)));
        currentPos = newPos;
      }
      activeSpans[ep.spanIndex()] = ep.risingEdge();
    }
    assert(currentPos == endpts.last().position());
    return overlaps.get();
  }

 private:
  // The span of this overlap
  Sp _span;

  // Indices to the original spans that overlap here.
  Array<T> _objects;
};

typedef SpanOverlap<int> SpanOverlapi;


} /* namespace sail */

#endif /* SPANOVERLAP_H_ */
