/*
 * Plot.h
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#ifndef SERVER_PLOT_PLOT_H_
#define SERVER_PLOT_PLOT_H_

#include <server/common/BBox.h>
#include <Eigen/Dense>
#include <server/common/AbstractArray.h>
#include <server/common/ArrayBuilder.h>

namespace sail {

class Plottable;

// Do we want to distinguish between lines and points?
enum PlotType {
  SolidLine,
  SolidCircle,
  Cross
};

struct RenderSettings {
  RenderSettings();

  Eigen::Vector3d rgbColor; // red, green, blue as values in [0, 1]

  // Not sure what the appropriate units should be.
  // If we are rendering on the screen, pixels make sense. On paper, maybe millimetres...
  // Maybe we can leave that a little bit vague for now.
  //
  // Both these options are useful. For instance, if we are drawing crosses,
  // lineWidth can be the widths of the two lines making up the cross and markerSize
  // how much space the cross occupies on the screen.
  double lineWidth;
  double markerSize;

  enum PlotType type;
};

/* Represents an abstract model of a plot. It can
 * for instance be an OpenGL context, a Gnuplot, a textfile with Matlab or Latex
 * code to render plots, etc.
 *
 * We could implement this by requiring the backend to support a small set of drawing primitives,
 * that the Plottable's can call. Or the Plottable can do a dynamic downcast of the backend to access
 * specific capabilities.
 */
class PlotModel {
public:
  virtual void render(
      const AbstractArray<Eigen::Vector3d> &x, const RenderSettings &s) = 0;

  virtual ~PlotModel() {}
};

/*
 * Implements something that can be plotted.
 */
class Plottable {
public:
  virtual BBox3d bbox() = 0;
  virtual void render(PlotModel *dst) = 0;

  typedef std::shared_ptr<Plottable> Ptr;
  virtual ~Plottable() {}
};

class PlottablePoints : public Plottable {
public:
  BBox3d bbox() {return _box;}

  PlottablePoints(
      const Array<Eigen::Vector3d> &pts,
      const RenderSettings &settings = RenderSettings());

  void render(PlotModel *dst);

  template <typename Coll>
  static Plottable::Ptr make(const Coll &coll, const RenderSettings &rs = RenderSettings()) {
    int n = coll.size();
    ArrayBuilder<Eigen::Vector3d> dst(n);
    for (auto x: coll) {
      dst.add(x);
    }
    return Plottable::Ptr(new PlottablePoints(dst.get(), rs));
  }
private:
  RenderSettings _rs;
  Array<Eigen::Vector3d> _pts;
  BBox3d _box;
};

class PlottableList {
public:
  template <typename T>
  void add(T x) {
    _objs.push_back(Plottable::Ptr(x));
  }

  BBox3d getBBox() {
    return _box;
  }
private:
  BBox<double, 3> _box;
  std::vector<Plottable::Ptr> _objs;

  template <typename Coll>
  void initialize(const Coll &coll) {
    for (auto x: coll) {
      add(x);
    }
  }
};

}

#endif /* SERVER_PLOT_PLOT_H_ */
