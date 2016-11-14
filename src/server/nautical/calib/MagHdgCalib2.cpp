/*
 * MagHdgCalib2.cpp
 *
 *  Created on: 13 Nov 2016
 *      Author: jonas
 */

#include <server/nautical/calib/MagHdgCalib2.h>
#include <server/math/QuadForm.h>
#include <server/nautical/common.h>
#include <server/math/Integral1d.h>
#include <server/plot/CairoUtils.h>
#include <server/plot/PlotUtils.h>
#include <cairo/cairo-svg.h>
#include <server/common/DomUtils.h>
#include <server/common/Functional.h>
#include <server/common/string.h>
#include <server/math/SineFit.h>

namespace sail {
namespace MagHdgCalib2 {

auto velocityUnit = 1.0_mps;
auto angleUnit = 1.0_rad;

template <typename T>
using Vec = Eigen::Matrix<T, 2, 1>;

template <typename T>
using QF = QuadForm<2, 1, T>;

template <typename T>
QF<T> makeQuadForm(HorizontalMotion<double> m,
    Angle<T> angle) {
  auto u = velocityUnit.cast<T>();
  Vec<T> v = makeNauticalUnitVector(angle);
  T normal[2] = {-v(1), v(0)};
  T b = T(normal[0]*(m[0]/u) + normal[1]*(m[1]/u));
  return QF<T>::fit(normal, &b);
}

template <typename T>
Array<QF<T>> makeQuadForms(
    const SplineGpsFilter::EcefCurve &gpsCurve,
    Array<TimedValue<Angle<T>>> headings, Angle<T> offset) {
  int n = headings.size();
  Array<QF<T>> dst(n);
  for (int i = 0; i < n; i++) {
    auto h = headings[i];
    dst[i] = makeQuadForm<T>(
        gpsCurve.evaluateHorizontalMotion(h.time),
        h.value + offset);
  }
  return dst;
}

template <typename T>
T computeMedian(Array<T> *src) {
  if (src->empty()) {
    LOG(FATAL) << "Please provide more data!";
    return NAN;
  }
  std::sort(src->begin(), src->end());
  return (*src)[src->middle()];
}

template <typename T>
Array<T> evaluateSlidingWindowCosts(
    const Integral1d<QF<T>> &itg, int windowSize) {
  int n = itg.size() - windowSize + 1;
  Array<T> dst(n);
  auto R = QF<T>::makeReg(1.0e-9);
  for (int i = 0; i < n; i++) {
    dst[i] = (R + itg.integrate(i,
        i + windowSize)).evalOpt2x1();
  }
  return dst;
}

template <typename T>
Sine::Sample evaluateFit(const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings, Angle<T> correction) {
  auto forms = makeQuadForms<T>(
      gpsCurve, headings, correction);
  Integral1d<QF<T>> itg(forms, QF<T>::makeReg(1.0e-9));
  auto costs = evaluateSlidingWindowCosts(itg,
      settings.windowSize);
  return Sine::Sample(correction,
      computeMedian(&costs), 1.0);
}


// sin(theta + phi)
// cos(theta)sin(phi) + sin(theta)*cos(phi)
//
// sin(phi) = xy[0]
// cos(phi) = xy[1]
// phi = atan2(xy[0], xy[1])
/*Angle<double> computePhase(
    const Array<std::pair<Angle<double>, double>> &src) {
  typedef QuadForm<3, 1> QF3;
  QF3 sum = QF3::makeReg(1.0e-9);
  for (auto x: src) {
    double xy[3] = {cos(x.first), sin(x.first), 1.0};
    double c = x.second;
    sum += QF3::fit(xy, &c);
  }
  auto xy = sum.minimize();
  if (xy.empty()) {
    return 0.0_deg;
  }
  return atan2(xy(0, 0), xy(1, 0))*1.0_rad;
}*/

Array<Sine::Sample> makeCurveToPlot(
    const SplineGpsFilter::EcefCurve &gpsCurve,
        const Array<TimedValue<Angle<double>>> &headings,
        const Settings &settings) {
  LOG(INFO) << "Make curve to plot for window size "
      << settings.windowSize;
  LineKM m(0, settings.sampleCount-1, -M_PI, M_PI);
  Array<Sine::Sample> dst(settings.sampleCount);
  for (int i = 0; i < settings.sampleCount; i++) {
    auto angle = m(i)*1.0_rad;
    dst[i] = evaluateFit<double>(
        gpsCurve, headings, settings,
        angle);
  }
  return dst;
}

Array<Sine::Sample> normalizeYByMax(
    const Array<Sine::Sample> &src) {
  double maxv = 0.0;
  for (auto v: src) {
    maxv = std::max(
        double(maxv), double(v.y));
  }
  Array<Sine::Sample> dst = src.dup();
  for (auto &x: dst) {
    x.y /= maxv;
  }
  return dst;
}

Vec<double> v2(const Sine::Sample &x) {
  return Vec<double>(x.angle.degrees(), x.y);
}

Array<Array<Vec<double>>> makeCurvesToPlot(
    const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Array<Settings> &settings) {
  int n = settings.size();
  Array<Array<Vec<double>>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = sail::map(normalizeYByMax(makeCurveToPlot(gpsCurve, headings,
        settings[i])), &v2);
  }
  return dst;
}

void makeAngleFitnessPlot(
    const SplineGpsFilter::EcefCurve &gpsCurve,
        const Array<TimedValue<Angle<double>>> &headings,
        const Array<Settings> &settings,
    DOM::Node *dst) {
  auto curvesToPlot = makeCurvesToPlot(
      gpsCurve, headings, settings);

  DOM::makeSubNode(dst,
      stringFormat("Number of mag hdg samples: %d",
          headings.size()));

  auto image = DOM::makeGeneratedImageNode(dst, ".svg");
  PlotUtils::Settings2d plotSettings;
  auto surface = Cairo::sharedPtrWrap(
      cairo_svg_surface_create(
          image.toString().c_str(), plotSettings.width,
          plotSettings.height));
  auto cr = Cairo::sharedPtrWrap(cairo_create(surface.get()));
  plotSettings.orthogonal = false;

  LineKM hueMap(0, settings.size()-1, 0.0, 240.0);
  Cairo::renderPlot(plotSettings, [&](cairo_t *dst) {
    for (int i = 0; i < settings.size(); i++) {
      auto curve = curvesToPlot[i];
      if (!curve.empty()) {
        Cairo::setSourceColor(dst, PlotUtils::HSV::fromHue(
            hueMap(i)*1.0_deg));

        Cairo::moveTo(dst, curve[0]);
        for (auto x: curve.sliceFrom(1)) {
          Cairo::lineTo(dst, x);
        }
        Cairo::WithLocalDeviceScale wlds(dst,
            Cairo::WithLocalDeviceScale::Identity);
        cairo_stroke(dst);
      }
    }
  }, "Correction (degrees)",
  "Objective function", cr.get());
}

void makeFittedSinePlot(
    const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings,
    DOM::Node *dst) {
  auto image = DOM::makeGeneratedImageNode(dst, ".svg");
  PlotUtils::Settings2d plotSettings;
  auto surface = Cairo::sharedPtrWrap(
      cairo_svg_surface_create(
          image.toString().c_str(), plotSettings.width,
          plotSettings.height));
  auto cr = Cairo::sharedPtrWrap(cairo_create(surface.get()));
  plotSettings.orthogonal = false;

  auto curve = makeCurveToPlot(gpsCurve, headings,
          settings);
  auto fittedSine0 = fit(2.0, curve);
  if (!fittedSine0.defined()) {
    LOG(ERROR) << "failed to fit sine";
    return;
  }
  auto fittedSine = fittedSine0.get();
  auto fittedSamples = fittedSine.sample(
      360.0, -180.0_deg, 180.0_deg);
  auto curvePts = sail::map(curve, &v2);
  auto fittedPts = sail::map(fittedSamples, &v2);

  Cairo::renderPlot(plotSettings, [&](cairo_t *dst) {
      Cairo::setSourceColor(
          dst, PlotUtils::HSV::fromHue(0.0_deg));
      Cairo::plotLineStrip(dst, curvePts);
      Cairo::setSourceColor(
          dst, PlotUtils::HSV::fromHue(240.0_deg));
      Cairo::plotLineStrip(dst, fittedPts);
    }, "Correction (degrees)",
    "Objective function", cr.get());
}

Optional<Angle<double>> optimizeSineFit(
    const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings) {
  auto curve = makeCurveToPlot(gpsCurve, headings,
          settings);
  auto fittedSine0 = fit(2.0, curve);
  if (!fittedSine0.defined()) {
    LOG(ERROR) << "Failed to fit sine";
    return Optional<Angle<double>>();
  }
  return minimize(fittedSine0.get()).smallest();
}


}
}

