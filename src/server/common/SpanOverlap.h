/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SPANOVERLAP_H_
#define SPANOVERLAP_H_

#include <server/common/Span.h>
#include <server/common/Array.h>

namespace sail {

class SpanOverlap {
 public:
  SpanOverlap() {}
  SpanOverlap(Spani span_, Arrayi indices_) : _span(span_), _indices(indices_) {}
  Spani span() const {return _span;}
  Arrayi indices() const {return _indices;}

  bool operator== (const SpanOverlap &other) const {
    return _span.minv() == other._span.minv() && _span.maxv() == other._span.maxv() && _indices == other._indices;
  }
 private:
  // The span of this overlap
  Spani _span;

  // Indices to the original spans that overlap here.
  Arrayi _indices;
};

Array<SpanOverlap> computeSpanOverlaps(Array<Spani> spans);

} /* namespace sail */

#endif /* SPANOVERLAP_H_ */
