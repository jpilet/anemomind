/*
 * SvgPlot.h
 *
 *  Created on: 13 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_PLOT_SVGPLOT_H_
#define SERVER_PLOT_SVGPLOT_H_

#include <server/common/HtmlLog.h>
#include <server/common/BBox.h>
#include <Eigen/Dense>
#include <server/common/Optional.h>


namespace sail {
namespace SvgPlot {

struct RenderSettings {
  std::string colorCode = "blue";
};

// Makes
class Plottable {
public:
  virtual void extend(BBox3d *dst) = 0;
  virtual void render2d(HtmlNode::Ptr dst) = 0;

  typedef std::shared_ptr<Plottable> Ptr;

  virtual ~Plottable();
};

class LineStrip : public Plottable {
public:

private:
  Array<Eigen::Vector3d> _pts;
};

struct Settings2d {
  static constexpr double masterMargin = 30;
  double xMargin = masterMargin*1.61803398875;
  double yMargin = masterMargin;
  double width = 640;
  double height = 480;
  bool axisIJ = false;
  bool orthogonal = true;
  Optional<BBox3d> roi;
};

// Collects all the objects
void render2d(
    const std::vector<Plottable::Ptr> &plottables,
    HtmlNode::Ptr dst,
    const Settings2d &settings);

}
}

#endif /* SERVER_PLOT_SVGPLOT_H_ */
