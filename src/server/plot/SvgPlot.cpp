/*
 * SvgPlot.cpp
 *
 *  Created on: 13 Oct 2016
 *      Author: jonas
 */

#include <server/plot/SvgPlot.h>
#include <server/common/math.h>
#include <server/common/LineKM.h>

namespace sail {
namespace SvgPlot {

BBox3d makeDefaultBBox() {
  BBox3d dst;
  double a[3] = {0, 0, 0};
  double b[3] = {1, 1, 1};
  dst.extend(a);
  dst.extend(b);
  return dst;
}

bool isEmpty(const BBox3d &box) {
  double diag = 0.0;
  for (int i = 0; i < 3; i++) {
    diag += sqr(box.getSpan(i).width());
  }
  return 1.0e-6 < sqrt(diag + 1.0e-6);
}

BBox3d computeBBox(
    const std::vector<Plottable::Ptr> &plottables,
    const Settings2d &settings) {
  if (settings.roi.defined()) {
    return settings.roi.get();
  }

  BBox3d bbox;
  for (auto obj: plottables) {
    obj->extend(&bbox);
  }
  if (isEmpty(bbox)) {
    bbox = makeDefaultBBox();
  }
  return bbox;
}

Eigen::Vector4d getCorner(const BBox3d &box, int cornerIndex0) {
  Eigen::Vector4d offset = Eigen::Vector4d::Zero();
  offset(3) = 1.0;
  int cornerIndex = cornerIndex0;
  for (int i = 0; i < 3; i++) {
    int rem = cornerIndex % 2;
    const auto &sp = box.getSpan(i);
    offset(i) = rem == 0? sp.minv() : sp.maxv();
    cornerIndex = cornerIndex/2;
  }
  assert(cornerIndex == 0);
  return offset;
}

BBox3d projectBBox(const Eigen::Matrix4d &pose, const BBox3d &box) {
  BBox3d dst;
  for (int i = 0; i < 8; i++) {
    Eigen::Vector4d corner = getCorner(box, i);
    Eigen::Vector4d transformed = pose*corner;
    dst.extend(transformed.data());
  }
  return dst;
}

Eigen::Matrix<double, 2, 4> computeTotalProjection(
    const BBox3d &a, const Settings2d &settings) {


  // Assuming homogeneous 3d coordinates (for future 3d rendering)
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();

  if (!settings.axisIJ) {
    pose(1, 1) = -1;
    pose(2, 2) = -1; // So that z = cross(x, y) instead of z = -cross(x, y)
  }
  auto proj = projectBBox(pose, a);

  LineKM rawXFit(proj.getSpan(0).minv(), proj.getSpan(0).maxv(),
      settings.xMargin, settings.width - settings.xMargin);
  LineKM rawYFit(proj.getSpan(1).minv(), proj.getSpan(1).maxv(),
      settings.yMargin, settings.height - settings.yMargin);


  // Assuming homogenous 3d coordinates. But here, the z coordinate will just
  // disappear.
  Eigen::Matrix<double, 2, 4> viewMat = Eigen::Matrix<double, 2, 4>::Zero();

  int tcol = 3;
  if (settings.orthogonal) {
    double minScale = std::min(rawXFit.getK(), rawYFit.getK());
    Eigen::Vector2d srcMiddle(
        proj.getSpan(0).middle(),
        proj.getSpan(1).middle());
    Eigen::Vector2d dstMiddle(
        0.5*settings.width,
        0.5*settings.height);

    Eigen::Vector2d translate = dstMiddle - minScale*srcMiddle;
    viewMat(0, 0) = minScale; viewMat(0, tcol) = translate(0);
    viewMat(1, 1) = minScale; viewMat(1, tcol) = translate(1);
  } else {
    viewMat(0, 0) = rawXFit.getK(); viewMat(0, tcol) = rawXFit.getM();
    viewMat(1, 1) = rawYFit.getK(); viewMat(1, tcol) = rawYFit.getM();
  }
  return viewMat*pose;
}

HtmlNode::Ptr makePlotCanvas(const HtmlNode::Ptr &dst,
    const Eigen::Matrix<double, 2, 4> &mat) {
  int last = mat.cols()-1;
  std::stringstream ss;
  ss.precision(17);
  ss << "matrix(";
  ss << mat(0, 0) << ",";
  ss << mat(1, 0) << ",";
  ss << mat(0, 1) << ",";
  ss << mat(1, 1) << ",";
  ss << mat(0, last) << ",";
  ss << mat(1, last) << ")";

  return HtmlTag::make(dst, "g", {
    {"transform", ss.str()}
  });
}

void renderPlottablesToCanvas(
    const std::vector<Plottable::Ptr> &plottables,
    const RenderingContext &context) {
  for (auto obj: plottables) {
    obj->render2d(context);
  }
}

Plottable::Ptr LineStrip::make3d(
    const Array<Eigen::Vector3d> &pts,
    const RenderSettings &rs) {
  return Plottable::Ptr(new LineStrip(pts, rs));
}

Plottable::Ptr LineStrip::make2d(
    const Array<Eigen::Vector2d> &pts,
    const RenderSettings &rs) {
  int n = pts.size();
  Array<Eigen::Vector3d> dst(n);
  for (int i = 0; i < n; i++) {
    auto xy = pts[i];
    dst[i] = Eigen::Vector3d(xy(0), xy(1), 0.0);
  }
  return make3d(dst, rs);
}

void LineStrip::extend(BBox3d *dst) const {
  for (auto pt: _pts) {
    dst->extend(pt.data());
  }
}

std::pair<std::string, AttribValue> vectorEffect(const RenderSize::SizeMode &x) {
  return std::pair<std::string, AttribValue>(
      "vector-effect",
        x == RenderSize::Global? "non-scaling-stroke" : "none");
}

void LineStrip::render2d(const RenderingContext &dst) const {
  std::stringstream points;
  for (auto pt: _pts) {
    points << pt(0) << "," << pt(1) << " ";
  }

  std::stringstream style;
  style << "fill:none;stroke:"
      << _rs.colorCode << ";stroke-width:" << _rs.lineWidth.value;

  auto p = HtmlTag::make(dst.localCanvas, "polyline", {
      {"points", points.str()},
      {"style", style.str()},
      vectorEffect(_rs.lineWidth.mode)
  });
}


  void render2d(
      const std::vector<Plottable::Ptr> &plottables,
      HtmlNode::Ptr dst,
      const Settings2d &settings) {
  auto bbox = computeBBox(plottables, settings);
  Eigen::Matrix<double, 2, 4> proj
    = computeTotalProjection(bbox, settings);
  auto svg = HtmlTag::make(dst, "svg", {
    {"width", settings.width},
    {"height", settings.height}
  });
  auto canvas = makePlotCanvas(svg, proj);
  RenderingContext context{proj, canvas, svg};
  renderPlottablesToCanvas(plottables, context);
}


}
}
