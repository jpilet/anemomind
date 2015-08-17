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

/*
 * This function is used to identify jointly stable
 * segments of data when having several signals.
 * For instance, when building a target speed table,
 * we have TWA, TWS and boat speed as the three signals.
 * Call them Y0, Y1, Y2 respectively. They are all indexed
 * by a common time X. In that case, we will pass
 * as xyPairs = ((X, Y0), (X, Y1), (X, Y2))
 * The optimize function will then proceed by simplifying
 * each signal by approximating it using piecewise constant
 * values. These simplifications are used to determine how
 * locally stable a signal is. Finally, a vector valued signal
 * is returned, that is also piecewise constant and where
 * each constant segment has a stability. Those segments
 * can be used as input to build a target speed table.
 */
Array<Segment> optimize(
    Array<std::pair<Arrayd, Arrayd> > xyPairs,
    int sampleCount, LineKM sampleToX, int segmentCount);

}
}

#endif /* SERVER_MATH_STABILITY_H_ */
