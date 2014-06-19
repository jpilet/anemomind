/*
 *  Created on: 2014-06-19
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SPANOVERLAPOBJECTS_H_
#define SPANOVERLAPOBJECTS_H_

#include <server/common/SpanOverlapObjects.h>

namespace sail {

template <typename T>
class SpanOverlapObjects {
 public:
  typedef SpanOverlapObjects<T> ThisType;

  SpanOverlapObjects() {}
  SpanOverlapObjects(Spani span_, Array<T> objs_) : _span(span_), _objs(objs_) {}

  static Array<ThisType> compute(Array<Spani> spans, Array<T> objs) {
    // For every span, there is a corresponding object.
    assert(spans.size() == objs.size());

    Array<SpanOverlap> overlaps = SpanOverlap::compute(spans);
    return overlaps.map<ThisType>([&](const SpanOverlap &o) {
      return ThisType(o.span(), objs.slice(o.indices()));
    });
  }

  Spani span() const {return _span;}
  Array<T> objects() const {return _objs;}
 private:
  Spani _span;
  Array<T> _objs;
};

}

#endif /* SPANOVERLAPOBJECTS_H_ */
