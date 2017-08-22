/*
 * ColorMap.cpp
 *
 *  Created on: 23 Oct 2016
 *      Author: jonas
 */

#include <server/plot/ColorMap.h>
#include <server/common/IntervalUtils.h>

namespace sail {

PlotUtils::HSV lambdaToHSV(double lambda0) {
  auto lambda = clamp(lambda0, 0.0, 1.0);
  return PlotUtils::HSV::fromHue(lambda*240.0_deg);
}

std::function<PlotUtils::HSV(TimeStamp)> makeTimeColorMap(
    TimeStamp from, TimeStamp to) {
  return [=](TimeStamp t) {
    return lambdaToHSV(computeLambda<TimeStamp>(from, to, t));
  };
}


}


