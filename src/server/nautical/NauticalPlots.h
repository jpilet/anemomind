/*
 *  Created on: 2014-03-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  V
 */

#ifndef VARIOUSPLOTS_H_
#define VARIOUSPLOTS_H_

#include <server/nautical/Nav.h>
#include <server/common/PeriodicHist.h>

namespace sail {

void plotHist(PeriodicHist hist);
Arrayb makeReliableNavMask(Array<Nav> navs);
void plotPolarAWAAndWatSpeed(Array<Array<Nav> > navs);
PeriodicHist makeAWAHist(Array<Nav> navs, int binCount);
PeriodicHist makeAWAWatSpeedSumHist(Array<Nav> navs, int binCount);
PeriodicHist makeAWAAWSSumHist(Array<Nav> navs, int binCount);

void plotPolarAWAAndAvgWatSpeed(Array<Nav> navs, int binCount);

} /* namespace mmm */

#endif /* VARIOUSPLOTS_H_ */
