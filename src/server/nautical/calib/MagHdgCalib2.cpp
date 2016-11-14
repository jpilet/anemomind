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
T computeLogSum(Array<T> costs) {
  T sum = T(0.0);
  int counter = 0;
  for (auto c: costs) {
    if (0 < c) {
      sum += log(c);
      counter++;
    }
  }
  return (1.0/(counter))*sum;
}

template <typename T>
Array<T> evaluateSlidingWindowCosts(
    const Integral1d<QF<T>> &itg, int windowSize,
    int relMinWindowCount) {
  if (itg.size() <= windowSize*relMinWindowCount) {
    return Array<T>();
  }
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
Optional<Sine::Sample> evaluateFit(const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings, Angle<T> correction) {
  auto forms = makeQuadForms<T>(
      gpsCurve, headings, correction);
  Integral1d<QF<T>> itg(forms, QF<T>::makeReg(1.0e-9));
  auto costs = evaluateSlidingWindowCosts(itg,
      settings.windowSize, settings.relMinSampleCount);
  if (costs.empty()) {
    return Optional<Sine::Sample>();
  }
  /*return Sine::Sample(correction,
      computeMedian(&costs), 1.0);*/
  return Sine::Sample(correction,
      computeLogSum(costs), 1.0);
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
      << settings.windowSize << " and mag hdg samples " << headings.size();
  LineKM m(0, settings.sampleCount-1, -M_PI, M_PI);
  Array<Sine::Sample> dst(settings.sampleCount);
  for (int i = 0; i < settings.sampleCount; i++) {
    auto angle = m(i)*1.0_rad;
    auto x = evaluateFit<double>(
        gpsCurve, headings, settings,
        angle);
    if (!x.defined()) {
      return Array<Sine::Sample>();
    }
    dst[i] = x.get();
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
  plotSettings.orthonormal = false;

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
  plotSettings.orthonormal = false;

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
  std::cout << "Make the curve" << std::endl;
  auto curve = makeCurveToPlot(gpsCurve, headings,
          settings);
  if (curve.empty()) {
    return Optional<Angle<double>>();
  }
  std::cout << "Fit it" << std::endl;
  auto fittedSine0 = fit(2.0, curve);
  std::cout << "Move on!" << std::endl;
  if (!fittedSine0.defined()) {
    LOG(ERROR) << "Failed to fit sine";
    return Optional<Angle<double>>();
  }
  return minimize(fittedSine0.get()).smallest();
}

void makeSpreadPlot(
    const SplineGpsFilter::EcefCurve &gpsCurve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings,
    DOM::Node *dst) {
  ArrayBuilder<std::pair<Duration<double>, Span<double>>> acc;
  int counter = 2;

  bool go = true;
  while (go) {
    Duration<double> totalDuration = 0.0_s;
    LineKM split(0, counter, 0, headings.size());
    Span<double> span;
    for (int j = 0; j < counter; j++) {
      int from = int(round(split(j)));
      int to = int(round(split(j+1)));
      auto subset = headings.slice(from, to);
      auto angle = optimizeSineFit(gpsCurve,
          subset, settings);
      if (!angle.defined()) {
        go = false;
      } else {
        span.extend(angle.get().degrees());
        totalDuration +=
            subset.last().time - subset.first().time;
      }
    }
    if (go) {
      acc.add({((1.0/counter)*totalDuration), span});
      counter++;
    }
  }
  auto bounds = acc.get();
  if (bounds.empty()) {
    return;
  }


  auto upper = sail::map(bounds,
      [](const std::pair<Duration<double>, Span<double>> &f) {
    return Vec<double>(f.first.minutes(), f.second.maxv());
  });
  auto lower = sail::map(bounds,
      [](const std::pair<Duration<double>, Span<double>> &f) {
    return Vec<double>(f.first.minutes(), f.second.minv());
  });

  PlotUtils::Settings2d settings2d;
  double margScale = 3;
  settings2d.xMargin *= margScale;
  settings2d.yMargin *= margScale;
  settings2d.width = 800;
  settings2d.height = 600;
  settings2d.orthonormal = false;
  auto image = Cairo::Setup::svg(
      DOM::makeGeneratedImageNode(dst, ".svg").toString(),
      settings2d.width, settings2d.height);

  Cairo::renderPlot(settings2d, [&](cairo_t *dst) {
    Cairo::plotLineStrip(dst, lower);
    Cairo::plotLineStrip(dst, upper);
  },  "Minutes (slice size)", "Bounds (degrees)", image.cr.get());

  auto final = bounds.first();
  DOM::addSubTextNode(dst, "p",
      stringFormat("Final error estimate for slice size of %s: %.3g degrees",
          final.first.str().c_str(),
          final.second.width()));

}


}
}

