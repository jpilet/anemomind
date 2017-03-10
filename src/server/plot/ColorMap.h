/*
 * ColorMap.h
 *
 *  Created on: 23 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_PLOT_COLORMAP_H_
#define SERVER_PLOT_COLORMAP_H_

#include <server/common/TimeStamp.h>
#include <server/plot/PlotUtils.h>

using namespace sail;

namespace sail {

std::function<PlotUtils::HSV(TimeStamp)> makeTimeColorMap(
    TimeStamp from, TimeStamp to);

}


#endif /* SERVER_PLOT_COLORMAP_H_ */
