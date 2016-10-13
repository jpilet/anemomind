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

struct Settings2d {
  double innerMargin = 10;
  double width = 640;
  double height = 480;
};

// Collects all the objects
class Model {
public:
  void render2d(
      HtmlNode::Ptr dst,
      const Settings2d &settings);

private:
  BBox3d _bbox;
  std::vector<Plottable::Ptr> _plottables;
};

}
}

#endif /* SERVER_PLOT_SVGPLOT_H_ */
