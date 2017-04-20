/*
 * CairoUtils.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#include <server/plot/CairoUtils.h>
#include <Eigen/Dense>
#include <server/plot/AxisTicks.h>
#include <iostream>
#include <cairo/cairo-svg.h>

namespace sail {
namespace Cairo {

std::shared_ptr<cairo_surface_t> sharedPtrWrap(cairo_surface_t *x) {
  return std::shared_ptr<cairo_surface_t>(x, &cairo_surface_destroy);
}

void setCairoDefaults(cairo_t* cr) {
  // So that, when we plot a GPS trajectory,
  // it doesn't look as if the GPS filter failed
  // to remove outliers due to using MITER joining.
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
}

Setup Setup::svg(const std::string &filename,
      double width, double height) {
  Setup dst;
  dst.surface = sharedPtrWrap(
        cairo_svg_surface_create(
            filename.c_str(),
            width, height));
  dst.cr = sharedPtrWrap(cairo_create(dst.surface.get()));
  setCairoDefaults(dst.cr.get());
  return dst;
}

std::shared_ptr<cairo_t> sharedPtrWrap(cairo_t *x) {
  return std::shared_ptr<cairo_t>(x, &cairo_destroy);
}

WithLocalContext::WithLocalContext(cairo_t *cr) : _cr(cr) {
  cairo_save(cr);
}

WithLocalContext::~WithLocalContext() {
  cairo_restore(_cr);
}

WithLocalDeviceScale::WithLocalDeviceScale(
    cairo_t *cr, Mode mode) : _cr(cr) {
  cairo_get_matrix(cr, &_backup);
  auto tmp = _backup;
  auto absDet = std::abs(tmp.xx*tmp.yy - tmp.xy*tmp.yx);
  if (mode == Mode::Identity || absDet < 1.0e-12) {
    tmp.xx = 1.0;
    tmp.yy = 1.0;
    tmp.xy = 0.0;
    tmp.yx = 0.0;
  } else if (mode == Mode::Determinant){
    double s = 1.0/sqrt(absDet);
    tmp.xx *= s;
    tmp.xy *= s;
    tmp.yx *= s;
    tmp.yy *= s;
  } else if (mode == Mode::SVD) {
    Eigen::Matrix2d A;
    A << tmp.xx, tmp.xy,
         tmp.yx, tmp.yy;
    Eigen::JacobiSVD<Eigen::Matrix2d> svd(A,
        Eigen::ComputeFullU | Eigen::ComputeFullV);
    CHECK(svd.computeU() && svd.computeV());
    // Recompose the matrix again, but without
    // the singular values:
    // that is, the singular values are all 1.0
    Eigen::Matrix2d B = svd.matrixU()*svd.matrixV().transpose();
    tmp.xx = B(0, 0);
    tmp.xy = B(0, 1);
    tmp.yx = B(1, 0);
    tmp.yy = B(1, 1);
  }
  cairo_set_matrix(cr, &tmp);
}
WithLocalDeviceScale::~WithLocalDeviceScale() {
  cairo_set_matrix(_cr, &_backup);
}


void setSourceColor(cairo_t *cr, const PlotUtils::RGB &rgb) {
  cairo_set_source_rgb(cr, rgb.red, rgb.green, rgb.blue);
}

void drawBoat(cairo_t *cr, double boatLength) {
  double cutoff = 0.5;
  double widthFactor = 1.5;

  WithLocalContext context(cr);
  cairo_reset_clip(cr);

  double h = boatLength/(1.0 + cutoff);
  double middleShift = 0.5*boatLength - h;
  double k = h*widthFactor;
  double r = sqrt(k*k + h*h);
  cairo_arc(cr, -k, -middleShift, r, 0.0, 2.0*M_PI);
  cairo_clip(cr);
  cairo_arc(cr, k, -middleShift, r, 0.0, 2.0*M_PI);
  cairo_clip(cr);
  cairo_rectangle(cr, -2.0*r, -cutoff*boatLength - middleShift, 4.0*r, 2*h);
  cairo_clip(cr);
  cairo_paint(cr);
}

void setSourceColor(cairo_t *cr, const PlotUtils::HSV &hsv) {
  setSourceColor(cr, PlotUtils::hsv2rgb(hsv));
}

void rotateMathematically(cairo_t *cr, Angle<double> angle) {
  cairo_rotate(cr, angle.radians());
}

void drawFilledCircle(cairo_t *cr,
    double r) {
  WithLocalContext context(cr);
  cairo_arc(cr, 0, 0, r, 0.0, 2.0*M_PI);
  cairo_clip(cr);
  cairo_paint(cr);
}

void rotateGeographically(cairo_t *cr, Angle<double> angle) {
  rotateMathematically(cr, -angle);
}

void deviceStroke(cairo_t *cr) {
  WithLocalDeviceScale context(cr, WithLocalDeviceScale::Identity);
  cairo_stroke(cr);
}

bool drawArrow(cairo_t *cr,
    const Eigen::Vector2d &from,
    const Eigen::Vector2d &to,
    double pointSize) {
  auto dir = (to - from);
  auto len = dir.norm();
  if (len < 1.0e-6) {
    return false;
  }
  Eigen::Vector2d xAxis = (1.0/len)*dir;
  Eigen::Vector2d yAxis(xAxis(1), -xAxis(0));
  Eigen::Vector2d yStep = yAxis*pointSize;
  Eigen::Vector2d base = to - xAxis*pointSize;
  Eigen::Vector2d pt0 = base - yStep;
  Eigen::Vector2d pt1 = base + yStep;

  cairo_move_to(cr, from(0), from(1));
  cairo_line_to(cr, to(0), to(1));

  // If the arrow is too small to be visible,
  // consider using WithLocalDeviceScale.
  cairo_stroke(cr);

  cairo_move_to(cr, pt0(0), pt0(1));
  cairo_line_to(cr, to(0), to(1));
  cairo_line_to(cr, pt1(0), pt1(1));
  cairo_stroke(cr);

  return true;
}

bool drawLocalFlow(
    cairo_t *cr,
    Eigen::Vector2d flowVector,
    double localStddev, int n,
    double pointSize,
    std::default_random_engine *rng) {
  std::normal_distribution<double> distrib(0.0, localStddev);
  Eigen::Vector2d half = 0.5*flowVector;
  for (int i = 0; i < n; i++) {
    Eigen::Vector2d at(distrib(*rng), distrib(*rng));
    if (!drawArrow(cr, at - half, at + half, pointSize)) {
      return false;
    }
  }
  return true;
}

template <int rows, int cols>
cairo_matrix_t toCairo(const Eigen::Matrix<double, rows, cols> &mat) {
  static_assert(2 <= rows, "Too few rows");
  static_assert(3 <= cols, "Too few cols");
  cairo_matrix_t dst;
  dst.xx = mat(0, 0);
  dst.xy = mat(0, 1);
  dst.yx = mat(1, 0);
  dst.yy = mat(1, 1);
  dst.x0 = mat(0, cols-1);
  dst.y0 = mat(1, cols-1);
  return dst;
}

template cairo_matrix_t toCairo<2, 3>(const Eigen::Matrix<double, 2, 3> &mat);
template cairo_matrix_t toCairo<2, 4>(const Eigen::Matrix<double, 2, 4> &mat);

namespace {
  BBox3d getBBox(cairo_surface_t *surface,
      double pixelsPerUnit) {
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    cairo_recording_surface_ink_extents(surface,
        &x, &y, &width, &height);

    x /= pixelsPerUnit;
    y /= pixelsPerUnit;
    width /= pixelsPerUnit;
    height /= pixelsPerUnit;

    BBox3d box;
    box.extend({x, y, 0.0});
    box.extend({x+width, y+height, 0.0});

    return box;
  }
}

void renderPlot(
    const PlotUtils::Settings2d &settings,
    std::function<void(cairo_t*)> dataRenderer,
    std::function<void(BBox3d, cairo_t*)> contextRenderer,
    cairo_t *dst) {

  auto surface = sharedPtrWrap(
      cairo_recording_surface_create(
          CAIRO_CONTENT_COLOR_ALPHA, nullptr));
  auto cr = sharedPtrWrap(cairo_create(surface.get()));
  cairo_scale(cr.get(),
      settings.pixelsPerUnit,
      settings.pixelsPerUnit);
  dataRenderer(cr.get());
  auto dataBbox = getBBox(surface.get(), settings.pixelsPerUnit);
  contextRenderer(dataBbox, cr.get());
  auto fullBbox = getBBox(surface.get(), 1.0);
  auto goodBbox = PlotUtils::ensureGoodBBox(fullBbox, settings);
  auto proj = PlotUtils::computeTotalProjection(
      goodBbox, settings);

  // Render to the real surface
  WithLocalContext wlc(dst);
  auto mat = toCairo(proj);
  cairo_transform(dst, &mat);
  cairo_scale(dst,
      settings.pixelsPerUnit,
      settings.pixelsPerUnit);
  dataRenderer(dst);
  contextRenderer(dataBbox, dst);
}

double getAxisPosition(Array<AxisTick<double>> ticks) {
  if (ticks.first().position <= 0 && 0 < ticks.last().position) {
    return 0;
  }
  return ticks.first().position;
}

void renderAxisText(int dim, const std::string &text,
    cairo_t *dst) {
  WithLocalContext wlc(dst);
  WithLocalDeviceScale wlds(dst,
      WithLocalDeviceScale::SVD);
  cairo_translate(dst, 0.0, 10.0*(2*dim - 1));
  WithLocalDeviceScale wlds2(dst,
      WithLocalDeviceScale::Identity);
  cairo_show_text(dst, text.c_str());
}

void renderAxis(int dim,
    Array<AxisTick<double>> ticks,
    const std::string &label,
    double position, cairo_t *dst) {
  WithLocalContext wlc(dst);
  cairo_set_line_width(dst, 1.0);
  Cairo::setSourceColor(dst, PlotUtils::RGB::black());
  if (dim == 1) {
    cairo_matrix_t mat;
    mat.xx = 0;
    mat.xy = 1;
    mat.yx = 1;
    mat.yy = 0;
    cairo_transform(dst, &mat);
  }
  cairo_move_to(dst, ticks.first().position, position);
  cairo_line_to(dst, ticks.last().position, position);
  {
    WithLocalDeviceScale wlds(dst,
        WithLocalDeviceScale::Identity);
    cairo_stroke(dst);
  }
  for (auto tick: ticks) {
    WithLocalContext wlc2(dst);
    cairo_translate(dst, tick.position, position);
    {
      WithLocalDeviceScale wlds(dst,
          WithLocalDeviceScale::SVD);
      cairo_move_to(dst, 0.0, -5.0);
      cairo_line_to(dst, 0.0, 5.0);
      cairo_stroke(dst);
    }
    renderAxisText(dim, tick.tickLabel, dst);
  }
  {
    WithLocalContext w2(dst);
    cairo_translate(dst, ticks.last().position, position);
    renderAxisText(dim, label, dst);
  }
}

void renderPlot(
    const PlotUtils::Settings2d &settings,
    std::function<void(cairo_t*)> dataRenderer,
    const std::string &xLabel,
    const std::string &yLabel,
    cairo_t *dst) {
  renderPlot(settings, dataRenderer,
      [&](const BBox3d &box, cairo_t *dst) {
    typedef BasicTickIterator Iter;
    auto xTicks = computeAxisTicks<Iter>(
            box.getSpan(0).minv(),
            box.getSpan(0).maxv(), Iter());
    auto yTicks = computeAxisTicks<Iter>(
            box.getSpan(1).minv(),
            box.getSpan(1).maxv(), Iter());

    renderAxis(0, xTicks, xLabel,
        getAxisPosition(yTicks), dst);
    renderAxis(1, yTicks, yLabel,
        getAxisPosition(xTicks), dst);

  }, dst);
}

void moveTo(cairo_t *dst, const Eigen::Vector2d &x) {
  cairo_move_to(dst, x(0), x(1));
}

void lineTo(cairo_t *dst, const Eigen::Vector2d &x) {
  cairo_line_to(dst, x(0), x(1));
}

void plotLineStrip(cairo_t *dst,
    const Array<Eigen::Vector2d> &src) {
  if (src.empty()) {
    return;
  }
  moveTo(dst, src[0]);
  for (auto x: src.sliceFrom(1)) {
    lineTo(dst, x);
  }
  WithLocalDeviceScale wlds(dst,
      WithLocalDeviceScale::Identity);
  cairo_stroke(dst);
}

void plotDots(cairo_t *dst,
    const Array<Eigen::Vector2d> &pts,
    double dotSize) {
  WithLocalContext wlc(dst);
  for (auto x: pts) {
    WithLocalContext wlc2(dst);
    cairo_translate(dst, x(0), x(1));
    WithLocalDeviceScale wlds(dst,
        WithLocalDeviceScale::SVD);
    drawFilledCircle(dst, dotSize);
  }
}


}
} /* namespace sail */
