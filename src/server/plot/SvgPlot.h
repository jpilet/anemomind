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

struct RenderSize {
  enum SizeMode {
    Local, // Specifies the size in the global coordinate system.
    Global // Specifies the size in the local coordinate system.
  };

  double value = 3.0;
  SizeMode mode = Global;
};

struct RenderSettings {
  std::string colorCode = "blue";
  RenderSize markerSize, lineWidth;
};

struct RenderingContext {
  Eigen::Matrix<double, 2, 4> localToGlobalProj;
  HtmlNode::Ptr localCanvas;
  HtmlNode::Ptr globalCanvas;
};

// Makes
class Plottable {
public:
  virtual void extend(BBox3d *dst) const = 0;
  virtual void render2d(const RenderingContext &dst) const = 0;

  typedef std::shared_ptr<Plottable> Ptr;

  virtual ~Plottable() {}
};

class LineStrip : public Plottable {
public:
  LineStrip(const Array<Eigen::Vector3d> &pts, const RenderSettings &rs =
      RenderSettings()) :
    _pts(pts), _rs(rs) {}

  static Plottable::Ptr make(
      const Array<Eigen::Vector3d> &pts,
      const RenderSettings &rs = RenderSettings());
  static Plottable::Ptr make(
      const Array<Eigen::Vector2d> &pts,
      const RenderSettings &rs = RenderSettings());

  void extend(BBox3d *dst) const;
  void render2d(const RenderingContext &dst) const;
private:
  RenderSettings _rs;
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
