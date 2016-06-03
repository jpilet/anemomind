/*
 * Plot.cpp
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 */

#include <server/plot/Plot.h>

namespace sail {


RenderSettings::RenderSettings() : rgbColor(0, 0, 0),
    lineWidth(1.0), markerSize(3.0), type(SolidLine) {}

PlottablePoints::PlottablePoints(const Array<Eigen::Vector3d> &pts,
    const RenderSettings &rs) : _pts(pts), _rs(rs) {
  for (auto x: pts) {
    _box.extend(x.data());
  }
}

void PlottablePoints::render(PlotModel *dst) {

  // TODO: replace this by IndexableWrap class of jo-array-wrappers2
  class AA : public AbstractArray<Eigen::Vector3d> {
  public:
    AA(const Array<Eigen::Vector3d> &src) : _src(src) {}

    Eigen::Vector3d operator[](int i) const override {
      return _src[i];
    }

    size_t size() const override {
      return _src.size();
    }
  private:
    const Array<Eigen::Vector3d> &_src;
  };

  dst->render(AA(_pts), _rs);
}


}
