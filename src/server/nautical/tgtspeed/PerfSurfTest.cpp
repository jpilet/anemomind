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
#include <server/common/string.h>

using namespace sail;

auto unit = 1.0_kn;

// A speed function that given wind speed returns the maximum boat
// speed. Just hypothetical, for testing.
Velocity<double> trueMaxSpeed(const Velocity<double>& src) {
  return unit*10.0*(1 - exp(double(-src/13.0_kn)));
}

std::default_random_engine rng(0);

// Used to produce a gradually varying signal, like a random walk.
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

// Convert the performance surface points
// to something convenient that we can plot.
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

Optional<Eigen::Vector2d> toNormed(const PerfSurfPt& x, const PerfSurfSettings& s) {
  double normed = double(x.boatSpeed/s.refSpeed(x.windVertexWeights));
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

Array<Eigen::Vector2d> levelsToCoords(
    const Eigen::VectorXd& X) {
  int n = X.size();
  Array<Eigen::Vector2d> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = Eigen::Vector2d(
        double(double(i)*resolution/unit),
        double(1.0/X(i)));
  }
  return dst;
}

TEST(PerfSurfTest, SumConstraintTest) {
  int n = 4;
  double expectedSum = 7.0;

  SumConstraint cst(n, expectedSum);

  Array<double> coeffs{9.34, 199.3444, 78.345};
  EXPECT_EQ(coeffs.size(), cst.coeffCount());

  double sum = 0.0;

  auto converted = cst.apply(coeffs);

  double sum2 = 0.0;
  for (int i = 0; i < n; i++) {
    auto x = cst.get(i);
    auto ptrs = x.pointers(coeffs.getData());
    auto value = x.eval(*ptrs.first, *ptrs.second);
    sum += value;
    sum2 += converted[i];
  }

  EXPECT_NEAR(sum, expectedSum, 1.0e-4);
  EXPECT_NEAR(sum2, expectedSum, 1.0e-4);
}

Array<Eigen::Vector2d> verticesToPlotData(
    const Array<Velocity<double>>& vertices) {
  int n = vertices.size();
  Array<Eigen::Vector2d> dst(n);
  for (int i = 0; i < n; i++) {
    auto windSpeed = decodeWindSpeed({{i, 1.0}});
    dst[i] = Eigen::Vector2d(windSpeed/unit, vertices[i]/unit);
  }
  return dst;
}

TEST(PerfSurfTest, TestIt2) {
  int dataSize = 6000;
  auto data = makeData(dataSize);
  int vertexCount = getRequiredVertexCount(data);

  PerfSurfSettings settings;
  settings.perfRegWeight = 10;
  settings.refSpeed = &decodeWindSpeed;

  auto page = DOM::makeBasicHtmlPage("Perf test", "", "results");
  DOM::addSubTextNode(&page, "h1", "Perf surf");


  PlotUtils::Settings2d ps;
  ps.orthonormal = false;


  auto results = optimizePerfSurf(
      data,
      generateSurfaceNeighbors1d(vertexCount),
      settings);

  EXPECT_EQ(results.performances.size(), data.size());

  double perfSum = 0.0;
  double maxPerf = results.performances[0];
  double minPerf = maxPerf;
  int goodCount = 0;
  for (int i = 0; i < results.performances.size(); i++) {
    auto x = results.performances[i];
    if (std::abs(x - data[i].performance) < 0.2) {
      goodCount++;
    }
    perfSum += x;
    maxPerf = std::max(maxPerf, x);
    minPerf = std::min(minPerf, x);
  }

  double goodFrac = double(goodCount)/results.performances.size();
  EXPECT_LT(0.5, goodFrac);
  std::cout << "Good frac: " << goodFrac << std::endl;

  std::cout << "Average perf: "
      << perfSum/results.performances.size() << std::endl;
  std::cout << "Perf sum: " << perfSum << std::endl;
  std::cout << "Max perf: " << maxPerf << std::endl;
  std::cout << "Min perf: " << minPerf << std::endl;

  std::cout << "Normalized vertices " << std::endl;
  for (auto v: results.normalizedVertices) {
    std::cout << " " << v;
  }
  std::cout << std::endl;
  std::cout << "DONE" << std::endl;


  const bool VISUALIZE = true;


  ///////// Visualize the results
  if (VISUALIZE) {
    DOM::addSubTextNode(&page, "h2", "Input data");
    auto im = DOM::makeGeneratedImageNode(&page, ".svg");
    auto p = Cairo::Setup::svg(
        im.toString(),
        ps.width,
        ps.height);
    Cairo::renderPlot(ps, [&](cairo_t* cr) {
      Cairo::plotDots(cr, dataToPlotPoints(data), 1);

      cairo_set_line_width(cr, 0.5);
      Cairo::setSourceColor(cr, PlotUtils::HSV::fromHue(254.0_deg));
      Cairo::plotLineStrip(cr, verticesToPlotData(results.vertices));

    }, "Wind speed", "Boat speed", p.cr.get());
  }
}
