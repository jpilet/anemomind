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


Array<Spani> recursiveTemporalSplit(Array<Nav> sortedNavs,
    double relativeThresh = 0.1, Duration<double> lowerThresh = Duration<double>::seconds(8));



}
#endif /* TEMPORALSPLIT_H_ */
