/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TEMPORALSPLIT_H_
#define TEMPORALSPLIT_H_

#include <server/nautical/Nav.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Span.h>

namespace sail {

Array<Spani> temporalSplit(Array<Nav> sortedNavs,
    double relativeThresh, Duration<double> lowerThresh);

Array<Spani> recursiveTemporalSplit(Array<Nav> sortedNavs,
    double relativeThresh, Duration<double> lowerThresh);



}
#endif /* TEMPORALSPLIT_H_ */
