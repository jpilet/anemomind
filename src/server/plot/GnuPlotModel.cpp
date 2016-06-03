/*
 * GnuPlotModel.cpp
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#include <server/plot/GnuPlotModel.h>
#include <server/common/logging.h>
#include <server/common/ArrayIO.h>

namespace sail {

GnuPlotModel::GnuPlotModel(int dims) : _dims(dims) {
  CHECK(dims == 2 || dims == 3);
}

namespace {

  MDArray2d pointsToMat(int dims, const AbstractArray<Eigen::Vector3d> &pts) {
    int n = pts.size();
    MDArray2d mat(n, dims);
    for (int i = 0; i < n; i++) {
      auto pt = pts[i];
      for (int j = 0; j < dims; j++) {
        mat(i, j) = pt(j);
      }
    }
    return mat;
  }

}

void GnuPlotModel::render(
    const AbstractArray<Eigen::Vector3d> &x,
    const RenderSettings &s) {

  switch (s.type) {
  case PlotType::SolidCircle:
  case PlotType::Cross:
    _plot.set_style("points");
    break;
  case PlotType::SolidLine:
    _plot.set_style("lines");
    break;
  default:
    _plot.set_style("lines");
  };

  _plot.plot(pointsToMat(_dims, x));
}

void GnuPlotModel::show() {
  _plot.show();
}

} /* namespace sail */
