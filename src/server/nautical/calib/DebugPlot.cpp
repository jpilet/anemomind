/*
 * DebugPlot.cpp
 *
 *  Created on: 2 Oct 2016
 *      Author: jonas
 */

#include <server/nautical/calib/DebugPlot.h>
#include <server/common/string.h>

namespace sail {

HtmlNode::Ptr makeSvg(
    HtmlNode::Ptr parent,
    double pixelWidth, double pixelHeight) {
  return HtmlTag::make(parent, "svg", {
          {"height", int(round(pixelHeight))},
          {"width", int(round(pixelWidth))}
      });
}

LineKM makeAxisFun(bool forward,
    const AxisMapping &m) {
  auto n = forward? 1.0 : 0.0;
  return LineKM(m.sourceRange.minv(), m.sourceRange.maxv(),
      m.margin + (1 - n)*m.targetWidth,
      m.margin + n*m.targetWidth);
}

HtmlNode::Ptr makePlotSpace(
    const HtmlNode::Ptr &parent,
    const LineKM &xmap,
    const LineKM &ymap) {
  return HtmlTag::make(parent, "g", {
        {"transform",
            stringFormat("matrix(%.3g, %.3g, %.3g, %.3g, %.3g, %.3g)",
                xmap.getK(), 0.0,
                0.0, ymap.getK(),
                xmap.getM(), ymap.getM())},
  });
}


HtmlNode::Ptr makePlotSpace(HtmlNode::Ptr parent,
                            AxisMode mode,
                            const AxisMapping &x,
                            const AxisMapping &y) {
  auto xmap = makeAxisFun(true, x);
  auto ymap = makeAxisFun(mode == AxisMode::IJ, y);
  return makePlotSpace(parent, xmap, ymap);
}






}

