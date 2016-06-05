/*
 * GnuPlotModel.h
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#ifndef SERVER_PLOT_GNUPLOTMODEL_H_
#define SERVER_PLOT_GNUPLOTMODEL_H_

#include <server/plot/Plot.h>
#include <server/plot/extra.h>

namespace sail {

class GnuPlotModel : public PlotModel {
public:
  GnuPlotModel(int dims);

  void render(
      const AbstractArray<Eigen::Vector3d> &x,
      const RenderSettings &s);

  void show();
private:
  GnuplotExtra _plot;
  int _dims;
};

} /* namespace sail */

#endif /* SERVER_PLOT_GNUPLOTMODEL_H_ */
