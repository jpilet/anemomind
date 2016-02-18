/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TEMPORALSPLIT_H_
#define TEMPORALSPLIT_H_

#include <server/nautical/Nav.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Span.h>
#include <iostream>

namespace sail {


Array<Spani> recursiveTemporalSplit(NavCollection sortedNavs,
    double relativeThresh = 0.1,
    Duration<double> lowerThresh =
        Duration<double>::seconds(8));

void dispTemporalRaceOverview(Array<Spani> spans,
    NavCollection navs, std::ostream *out = &(std::cout));

}
#endif /* TEMPORALSPLIT_H_ */
