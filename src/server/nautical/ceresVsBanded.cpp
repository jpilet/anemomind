/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <gtest/gtest.h>
#include <server/nautical/GpsFilter.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/plot/extra.h>
#include <server/math/nonlinear/Ceres1dSolver.h>
#include <server/common/Span.h>

using namespace sail;

Array<Nav> getPsarosTestData2() {
  auto p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("psaros33_Banque_Sturdza")
    .pushDirectory("2014")
    .pushDirectory("20140821").get();
  auto navs = scanNmeaFolder(p, Nav::debuggingBoatId());
  std::sort(navs.begin(), navs.end());
  return navs;
}





  /*
   * ceres::DENSE_NORMAL_CHOLESKY
   * ceres::ITERATIVE_SCHUR;
   * ceres::SPARSE_NORMAL_CHOLESKY;
   * ceres::SPARSE_SCHUR
   * ceres::CGNR
   * ceres::ITERATIVE_SCHUR;
   *
   *
   *
   *
   *
   */


struct LabeledSettings {
  std::string label;
  int maxNavs;
  GpsFilter::Settings settings;
};


LabeledSettings makeCeresSettings(ceres::LinearSolverType solverType,
    std::string label) {
    const int iters = 30;
    LabeledSettings s;
    s.label = stringFormat("Ceres (%d iters) %s", iters, label.c_str());
    s.maxNavs = 1000;
    s.settings.useCeres = true;
    s.settings.ceresSolverType = solverType;
    return s;
}

LabeledSettings makeBandedSettings(int iters) {
  LabeledSettings s;
  s.label = stringFormat("Banded solver (%d iters)", iters);
  s.settings.filterSettings.iters = iters;
  return s;
}

Array<LabeledSettings> makeAllSettingsToTest() {
  ceres::LinearSolverType types[6] = {
      ceres::DENSE_NORMAL_CHOLESKY,
      ceres::DENSE_QR,
      ceres::SPARSE_NORMAL_CHOLESKY,
      ceres::SPARSE_SCHUR,
      ceres::ITERATIVE_SCHUR,
      ceres::CGNR,
  };
  std::string labels[6] = {
      "Dense Cholesky",
      "Dense QR",
      "Sparse normal Cholesky",
      "Sparse Schur",
      "Iterative Schur",
      "CGNR"
      };
  Array<LabeledSettings> settings(8);
  for (int i = 0; i < 6; i++) {
    settings[i] = makeCeresSettings(types[i], labels[i]);
  }
  settings[6] = makeBandedSettings(8);
  settings[7] = makeBandedSettings(30);
  return settings;
}

struct TimeData {
  LabeledSettings settings;
  Arrayi counts;
  Array<Duration<double> > times;

  MDArray2d makePlotData() const;
  MDArray2d makeLogPlotData() const;
};

MDArray2d TimeData::makePlotData() const {
  assert(counts.size() == times.size());
  int n = counts.size();
  MDArray2d data(n, 2);
  for (int i = 0; i < n; i++) {
    data(i, 0) = counts[i];
    data(i, 1) = times[i].seconds();
  }
  return data;
}

double log10(double x) {
  return log(x)/log(10.0);
}

MDArray2d TimeData::makeLogPlotData() const {
  auto data = makePlotData();
  for (int i = 0; i < data.rows(); i++) {
    data(i, 1) = log10(data(i, 1));
  }
  return data;
}

TimeData measureTime(
    Arrayi navCounts,
    Array<Nav> navs,
    LabeledSettings settings) {
  int n = navCounts.size();
  int counter = 0;
  Array<Duration<double> > durs(n);
  for (int i = 0; i < n; i++) {
    int navCount = navCounts[i];
    std::cout << EXPR_AND_VAL_AS_STRING(navCount) << std::endl;
    if (navCount < settings.maxNavs) {
      auto start = TimeStamp::now();
      auto results = GpsFilter::filter(navs.sliceTo(navCount), settings.settings);
      std::cout << "DONE optimizing";
      durs[i] = TimeStamp::now() - start;
      counter++;
    } else {
      break;
    }
  }
  return TimeData{settings, navCounts.sliceTo(counter), durs.sliceTo(counter)};
}

// Check that the filtered signal is reasonbly close to the non-filtered one.
TEST(GpsFilterTest, Comparison) {
  auto navs = getPsarosTestData2();

  int navCounts = 20;
  LineKM navSampleCountMap(0, navCounts-1, log(50), log(3500));

  Arrayi counts = Spani(0, navCounts).map<int>([&](int i){
    return int(floor(exp(navSampleCountMap(i))));
  });

  Array<LabeledSettings> settings = makeAllSettingsToTest().sliceFrom(6);
  int n = settings.size();
  Array<TimeData> timeData(n);
  for (int i = 0; i < n; i++) {
    std::cout << "OPTIMIZING FOR SETTINGS " << i+1 << "/" << settings.size() << std::endl;
    timeData[i] = measureTime(counts, navs, settings[i]);
  }


  {
    GnuplotExtra plot;
    plot.set_style("lines");
    plot.set_xlabel("Number of navs");
    plot.set_ylabel("Time (seconds)");
    for (auto data: timeData) {
      auto d = data.makePlotData();
      plot.plot(d, data.settings.label);
    }
    plot.show();
  }{
    GnuplotExtra plot;
    plot.set_style("lines");
    plot.set_xlabel("Number of navs");
    plot.set_ylabel("Log10( Time(seconds) )");
    for (auto data: timeData) {
      plot.plot(data.makePlotData(), data.settings.label);
    }
    plot.show();
  }
}
