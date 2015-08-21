/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
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
  int maxNavs = 3000000;
  GpsFilter::Settings settings;
};


LabeledSettings makeCeresSettings(ceres::LinearSolverType solverType,
    std::string label) {
    const int iters = 30;
    LabeledSettings s;
    s.label = stringFormat("Ceres (%d iters) %s", iters, label.c_str());
    s.maxNavs = 40000;
    s.settings.useCeres = true;
    s.settings.ceresSolverType = solverType;
    s.settings.filterSettings.iters = iters;
    s.settings.filterSettings.lambda = 0.1;
    return s;
}

LabeledSettings makeBandedSettings(int iters) {
  LabeledSettings s;
  s.label = stringFormat("Banded solver (%d iters)", iters);
  s.settings.filterSettings.iters = iters;
  s.settings.filterSettings.lambda = 0.1;
  return s;
}

LabeledSettings makeCeresSettings(int i) {
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
  return makeCeresSettings(types[i], labels[i]);
}

Array<LabeledSettings> makeAllSettingsToTest() {
  return Array<LabeledSettings>{
    makeBandedSettings(8),
    makeBandedSettings(30),
    makeCeresSettings(2)
  };
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
    if (navCount < settings.maxNavs) {
      auto start = TimeStamp::now();
      auto results = GpsFilter::filter(navs.sliceTo(navCount), settings.settings);
      durs[i] = TimeStamp::now() - start;
      counter++;
    } else {
      break;
    }
  }
  return TimeData{settings, navCounts.sliceTo(counter), durs.sliceTo(counter)};
}

// Check that the filtered signal is reasonbly close to the non-filtered one.
int main() {
  auto navs = getPsarosTestData2();

  int navCounts = 20;
  LineKM navSampleCountMap(0, navCounts-1, log(50), log(3500));

  Arrayi counts = Spani(0, navCounts).map<int>([&](int i){
    return int(floor(exp(navSampleCountMap(i))));
  });

  Array<LabeledSettings> settings = makeAllSettingsToTest();
  int n = settings.size();
  Array<TimeData> timeData(n);
  for (int i = 0; i < n; i++) {
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
      int last = d.rows()-1;
      std::cout << " " << data.settings.label << " with " <<
          d(last, 0) << " navs takes " << d(last, 1)
            << " seconds." << std::endl;
    }
    plot.show();
  }
}
