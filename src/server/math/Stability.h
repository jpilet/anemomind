/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  To identify stable segments of data that can be used to build accurate
 *  target speed tables. Each segment has a stability score.
 */

#ifndef SERVER_MATH_STABILITY_H_
#define SERVER_MATH_STABILITY_H_

#include <server/common/Span.h>
#include <server/common/LineKM.h>

namespace sail {
namespace Stability {

struct Segment {
 Spani span;
 double stability;
 Arrayd values;

 double totalStability() const {
   return span.width()*stability;
 }
};

Array<Segment> optimize(
    Array<std::pair<Arrayd, Arrayd> > xyPairs,
    int sampleCount, LineKM sampleToX, int segmentCount);

}
}

#endif /* SERVER_MATH_STABILITY_H_ */
