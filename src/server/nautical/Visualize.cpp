/*
 * Visualize.cpp
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#include <server/nautical/Visualize.h>

namespace sail {

RenderSettings makeDefaultTrajectoryRenderSettings() {
  RenderSettings rs;
  rs.rgbColor = Eigen::Vector3d(0, 1, 0);
  rs.lineWidth = 1.0;
  rs.type = PlotType::SolidLine;
  return rs;
}

Plottable::Ptr makeTrajectoryPlot(
    const GeographicReference &geoRef,
    const SampledSignal<GeographicPosition<double> > &positions,
    const RenderSettings &rs) {

  ArrayBuilder<Eigen::Vector3d> pts(positions.size());
  for (auto timeAndPos: positions) {
    auto xy = geoRef.map(timeAndPos.value);
    pts.add(Eigen::Vector3d(xy[0].meters(), xy[1].meters(), 0.0));
  }
  return PlottablePoints::make(pts.get(), rs);
}


} /* namespace sail */
