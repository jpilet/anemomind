/*
 * DebugPlot.cpp
 *
 *  Created on: 2 Oct 2016
 *      Author: jonas
 */

#include <server/nautical/calib/DebugPlot.h>

namespace sail {

HtmlNode::Ptr makePlotSpace(AxisMode mode,
                            const AxisMapping &x,
                            const AxisMapping &y) {
  LineKM xMap(x.sourceRange.minv(), x.sourceRange.maxv(),
      x.margin, x.margin + x.targetWidth);
  LineKM yMap();
}






}

