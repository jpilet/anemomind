/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "SpanOverlap.h"
#include <algorithm>

namespace sail {
namespace SpanOverlapImplementation {

  Array<EndPoint> listSortedEndPts(Array<Spani> spans) {
    int endptCount = 2*spans.size();
    Array<EndPoint> endpts(endptCount);
    for (int spanIndex = 0; spanIndex < spans.size(); spanIndex++) {
      int at = 2*spanIndex;
      Spani span = spans[spanIndex];
      endpts[at + 0] = EndPoint(spanIndex, span.minv(), true);
      endpts[at + 1] = EndPoint(spanIndex, span.maxv(), false);
    }
    std::sort(endpts.begin(), endpts.end());
    return endpts;
  }
}
} /* namespace sail */
