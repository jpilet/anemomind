/*
 * RealPerfSurf.h
 *
 *  Created on: 2 Mar 2018
 *      Author: jonas
 *
 * This is glue code needed to perform a full polar computation
 * on a dataset.
 *
 */

#ifndef SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_
#define SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_

#include <server/common/Transducer.h>
#include <iostream>
#include <server/nautical/NavDataset.h>
#include <server/common/Functional.h>

namespace sail {

struct RealPerfSurfSettings {
  Duration<double> timedTupleThreshold = 4.0_seconds;
  Duration<double> sessionGap = 1.0_minutes;
  std::function<bool(TimeStamp)> timeFilter = constantly(true);
};

struct RealPerfSurfResults {};

RealPerfSurfResults optimizeRealPerfSurf(
    const NavDataset& src,
    const RealPerfSurfSettings& settings);

} /* namespace sail */

#endif /* SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_ */
