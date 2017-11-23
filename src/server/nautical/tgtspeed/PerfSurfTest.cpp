/*
 * PerfSurfTest.cpp
 *
 *  Created on: 12 Nov 2017
 *      Author: jonas
 */

#include <server/nautical/tgtspeed/PerfSurf.h>
#include <server/common/ArrayBuilder.h>
#include <random>
#include <server/common/math.h>
#include <gtest/gtest.h>
#include <server/plot/CairoUtils.h>
#include <stdlib.h>
#include <server/common/logging.h>
#include <server/common/LineKM.h>
#include <server/common/DOMUtils.h>

using namespace sail;

auto unit = 1.0_kn;

// A speed function that given wind speed returns the maximum boat
// speed. Just hypothetical, for testing.
Velocity<double> trueMaxSpeed(const Velocity<double>& src) {
  return unit*10.0*(1 - exp(double(-src/13.0_kn)));
}

std::default_random_engine rng(0);

struct SmoothGen {
  SmoothGen(double maxVal, double maxChange)
    : _distrib(-maxChange, maxChange),
      _maxVal(maxVal) {}

  double operator()(double x) {
    return clamp<double>(x + _distrib(rng), 0, _maxVal);
  }
private:
  std::uniform_real_distribution<double> _distrib;
  double _maxVal;
};

Velocity<double> resolution = 0.5_mps;

// Represent the wind as a linear combination of some "vertices"
Array<WeightedIndex> encodeWindSpeed(const Velocity<double>& v) {
  double k = v/resolution;
  double fi = floor(k);
  int i = int(fi);
  double lambda = k - fi;
  return {{i, 1.0 - lambda}, {i+1, lambda}};
}

Velocity<double> decodeWindSpeed(const Array<WeightedIndex>& w) {
  Velocity<double> sum = 0.0_kn;
  for (auto x: w) {
    sum += x.weight*x.index*resolution;
  }
  return sum;
}

TEST(PerfSurfTest, WindCoding) {
  auto y = 12.34_mps;
  EXPECT_NEAR(decodeWindSpeed(encodeWindSpeed(y)).knots(), y.knots(), 1.0e-5);
}

std::ostream& operator<<(std::ostream& s, const PerfSurfPt& pt) {
  s << "perf=" << pt.performance << " time=" << pt.time.toIso8601String() <<
      " boat-speed=" << pt.boatSpeed.knots() << "kn wind-speed"
      << decodeWindSpeed(pt.windVertexWeights).knots() << "kn";
  return s;
}

TEST(PerfSurfTest, ConstraintedSolve) {
  Eigen::MatrixXd A(1, 2);
  A << 1, 0;
  Eigen::MatrixXd AtA = A.transpose()*A;

  Eigen::VectorXd X = solveConstrained(AtA, SystemConstraintType::Norm1);
  if (X(1) < 0) {
    X = -X;
  }
  EXPECT_NEAR(X(0), 0.0, 1.0e-6);
  EXPECT_NEAR(X(1), 1.0, 1.0e-6);

  Eigen::VectorXd Y = solveConstrained(AtA, SystemConstraintType::Sum1);
  std::cout << "Y = \n" << Y << std::endl;
  EXPECT_NEAR(Y(0), 0.0, 1.0e-6);
  EXPECT_NEAR(Y(1), 1.0, 1.0e-6);
}

Array<PerfSurfPt> makeData(int n) {
  double perf = 0;
  Velocity<double> maxWind = 15.0_mps;
  Velocity<double> wind = 0.0_kn;
  TimeStamp offset = TimeStamp::UTC(2017, 11, 12, 15, 1, 0);
  Duration<double> stepSize = 0.1_s;
  Array<PerfSurfPt> pts(n);
  SmoothGen perfGen(1.0, 0.025);
  SmoothGen windGen(16.0_mps/unit, 0.1_mps/unit);
  for (int i = 0; i < n; i++) {
    TimeStamp time = offset + double(i)*stepSize;

    auto maxSpeed = trueMaxSpeed(wind);

    PerfSurfPt pt;
    pt.time = time;
    pt.performance = perf;
    pt.boatSpeed = perf*maxSpeed;
    pt.windVertexWeights = encodeWindSpeed(wind);

    pts[i] = pt;

    perf = perfGen(perf);
    wind = windGen(wind/unit)*unit;
  }
  return pts;
}

Array<Eigen::Vector2d> dataToPlotPoints(
    const Array<PerfSurfPt>& pts) {
  int n = pts.size();
  Array<Eigen::Vector2d> dst(n);
  for (int i = 0; i < n; i++) {
    auto pt = pts[i];
    dst[i] = Eigen::Vector2d(
        decodeWindSpeed(pt.windVertexWeights)/unit,
        pt.boatSpeed/unit);
  }
  return dst;
}

int getRequiredVertexCount(const Array<PerfSurfPt>& pts) {
  int n = 0;
  for (const auto& pt: pts) {
    for (const auto& w: pt.windVertexWeights) {
      n = std::max(n, w.index+1);
    }
  }
  return n;
}

Array<Velocity<double>> initializeVertices(int n) {
  Array<Velocity<double>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = double(i)*resolution;
  }
  return dst;
}


Array<Eigen::Vector2d> solutionToCoords(
    const Array<Velocity<double>>& src) {
  int n = src.size();
  Array<Eigen::Vector2d> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = Eigen::Vector2d(
        double(double(i)*resolution/unit),
        double(src[i]/unit));
  }
  return dst;
}

void displaySolution(
    const Array<PerfSurfPt>& data,
    const Array<Array<Velocity<double>>>& optimized) {
  int solutionCount = optimized.size();

  auto hue = LineKM(0, solutionCount-1, 240.0, 360.0);

  PlotUtils::Settings2d plotSettings;
  auto p = Cairo::Setup::svg(
      "input_data.svg",
      plotSettings.width, plotSettings.height);
  Cairo::renderPlot(plotSettings, [&](cairo_t* cr) {
    for (int i = 0; i < solutionCount; i++) {
      LOG(INFO) << "Plot solution";
      Cairo::setSourceColor(cr, PlotUtils::HSV::fromHue(hue(i)*1.0_deg));
      Cairo::plotLineStrip(cr, solutionToCoords(optimized[i]));
    }
    Cairo::setSourceColor(cr, PlotUtils::RGB::black());
    Cairo::plotDots(
        cr, dataToPlotPoints(data), 1.0);
  }, "Wind speed", "Boat speed", p.cr.get());
}

Velocity<double> referenceSpeed(const PerfSurfPt& pt) {
  return decodeWindSpeed(pt.windVertexWeights);
}

Optional<Eigen::Vector2d> toNormed(const PerfSurfPt& x, const PerfSurfSettings& s) {
  double normed = double(x.boatSpeed/s.refSpeed(x));
  if (std::isfinite(normed) && 0 <= normed && normed < s.maxFactor) {
    return Eigen::Vector2d(
        decodeWindSpeed(x.windVertexWeights).knots(),
        normed);
  }
  return {};
}

Array<Eigen::Vector2d> normalizedDataToPlotPoints(
    const Array<PerfSurfPt>& src,
    const PerfSurfSettings& s) {
  int n = src.size();
  ArrayBuilder<Eigen::Vector2d> dst(n);
  for (int i = 0; i < n; i++) {
    auto x = toNormed(src[i], s);
    if (x.defined()) {
      dst.add(x.get());
    }
  }
  return dst.get();
}

TEST(PerfSurfTest, SpanTest) {
  auto spans = generatePairs({{1, 6}}, 4);
  EXPECT_EQ(spans.size(), 1);
  auto sp = spans[0];
  EXPECT_EQ(sp.first, 1);
  EXPECT_EQ(sp.second, 5);
}

TEST(PerfSurfTest, TestIt1) {
  int dataSize = 6000;
  auto data = makeData(dataSize);
  int vc = getRequiredVertexCount(data);
  auto vertices = initializeVertices(vc);

  PerfSurfSettings settings;
  settings.refSpeed = &referenceSpeed;

  auto page = DOM::makeBasicHtmlPage("Perf test", "", "results");
  DOM::addSubTextNode(&page, "h1", "Perf surf");

  PlotUtils::Settings2d ps;
  ps.orthonormal = false;

  auto pairs = generatePairs({{0, data.size()}}, 5);

  if (false) {
    DOM::addSubTextNode(&page, "h2", "Input data");
    auto im = DOM::makeGeneratedImageNode(&page, ".svg");
    auto p = Cairo::Setup::svg(
        im.toString(),
        ps.width,
        ps.height);
    Cairo::renderPlot(ps, [&](cairo_t* cr) {
      Cairo::plotDots(cr, dataToPlotPoints(data), 1);
    }, "Wind speed", "Boat speed", p.cr.get());
  }

  auto A = makeOneDimensionalReg(vc, 1);
  std::cout << "A = \n" << A << std::endl;

  if (true) {
    DOM::addSubTextNode(&page, "h2",
        "Divide all the boatspeeds by the reference speed");
    auto im = DOM::makeGeneratedImageNode(&page, ".svg");
    auto p = Cairo::Setup::svg(
        im.toString(),
        ps.width,
        ps.height);
    Cairo::renderPlot(ps, [&](cairo_t* cr) {
      auto pts = normalizedDataToPlotPoints(data, settings);
      Cairo::plotDots(cr, pts, 1);

      Cairo::setSourceColor(cr, PlotUtils::HSV::fromHue(240.0_deg));
      cairo_set_line_width(cr, 1.0);
      for (auto p: pairs) {
        auto a = toNormed(data[p.first], settings);
        auto b = toNormed(data[p.second], settings);
        if (a.defined() && b.defined()) {
          Cairo::plotLineStrip(cr, {a.get(), b.get()});
        }
      }
    }, "Wind speed", "Boat speed", p.cr.get());
  }
}
