/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/Stability.h>
#include <server/common/SpanOverlap.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/LineKM.h>
#include <server/math/PiecewisePolynomials.h>
#include <server/common/Functional.h>

namespace sail {
namespace Stability {

namespace {

struct Value {
 int signalIndex;
 double value;
 double stability;
};

void getSpansAndValues(int signalIndex,
    Arrayd X, Arrayd Y, int sampleCount, LineKM sampleToX, int segmentCount,
    ArrayBuilder<Spani> *spansOut, ArrayBuilder<Value> *valuesOut) {
    auto pieces = PiecewisePolynomials::optimizeForSegmentCount<1>(X, Y, sampleCount, sampleToX, segmentCount);
    int count = pieces.size();
    for (int i = 0; i < count; i++) {
      auto &piece = pieces[i];
      (*spansOut).add(piece.span);
      (*valuesOut).add(Value{signalIndex, piece.constantValue(),
        double(piece.span.width())});
    }
}

Segment makeSegmentFromOverlap(SpanOverlap<Value> overlap) {
  auto objs = overlap.objects();
  auto n = objs.size();
  Arrayd values(n);
  double stability = std::numeric_limits<double>::infinity();
  for (int i = 0; i < n; i++) {
    auto obj = objs[i];
    stability = std::min(stability, obj.stability);
    values[obj.signalIndex] = obj.value;
  }
  return Segment{overlap.span(), stability, values};
}

}


Array<Segment> optimize(Array<std::pair<Arrayd, Arrayd> > xyPairs,
    int sampleCount, LineKM sampleToX, int segmentCount) {
  ArrayBuilder<Spani> spans;
  ArrayBuilder<Value> values;
  for (int i = 0; i < xyPairs.size(); i++) {
    auto &xy = xyPairs[i];
    getSpansAndValues(i, xy.first, xy.second,
        sampleCount, sampleToX, segmentCount,
        &spans, &values);
  }
  return toArray(map(makeSegmentFromOverlap, SpanOverlap<Value>::compute(
      spans.get(), values.get())));

}



}
}
