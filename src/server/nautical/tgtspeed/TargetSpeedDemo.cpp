/*
 *  Created on: 2015-07-31
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/nautical/TestdataNavs.h>
#include <server/plot/extra.h>
#include <server/common/Span.h>
#include <server/nautical/FilteredNavData.h>
#include <device/Arduino/libraries/Corrector/Corrector.h>
#include <server/plot/extra.h>
#include <fstream>
#include <server/math/ADFunction.h>
#include <server/math/armaadolc.h>
#include <ceres/ceres.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/nonlinear/LevmarSettings.h>
#include <server/common/ArrayIO.h>
#include <server/nautical/TemporalSplit.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/logging.h>
#include <server/nautical/tgtspeed/TargetSpeedParam.h>
#include <server/nautical/tgtspeed/TargetSpeedPoint.h>
#include <server/nautical/tgtspeed/table.h>
#include <server/common/ScopedLog.h>
#include <server/nautical/tgtspeed/TargetSpeedSolver.h>
#include <server/common/ArgMap.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/Functional.h>

using namespace sail;

namespace {



  MDArray2d makePlotData(Array<TargetSpeedPoint> x) {
    int n = x.size();
    MDArray2d pts(n, 3);
    for (int i = 0; i < n; i++) {
      auto xy = x[i].wind();
      pts(i, 0) = xy[1].knots();
      pts(i, 1) = xy[0].knots();
      pts(i, 2) = x[i].boatSpeed().knots();
    }
    return pts;
  }

  Array<TargetSpeedPoint> getPolarNavData(Corrector<double> corr, FilteredNavData fdata) {
    return
        filter(
            sail::map(fdata.makeIndexedInstrumentAbstractions(), [&](const FilteredNavData::Indexed &x) {
      return TargetSpeedPoint(corr.correct(x));
    }), [&](const TargetSpeedPoint &x) {return x.defined();});
  }

  struct Settings {
    Settings();
    TargetSpeedSolver::Settings solverSettings;
    TargetSpeedParam param;
    std::string prefix;
  };



  Settings::Settings() : param(true, false, 24, 27, Velocity<double>::knots(26)),
      prefix("/tmp/tgtspeed") {}


  typedef std::function<Array<Nav>()> NavLoader;

  Array<TargetSpeedPoint> getPolarNavDataFromRawNavs(std::string cacheFilename,
      NavLoader loader) {
    auto data = loadRawArray<TargetSpeedPoint>(cacheFilename);
    if (data.empty()) {
      auto navs = loader();
      std::cout << "No cached data found in file " << cacheFilename << std::endl;
      double lowerThresh = 8;
      double relativeThresh = 0.1;

      Array<Spani> spans = recursiveTemporalSplit(navs,
          relativeThresh, Duration<double>::seconds(lowerThresh));

      std::cout << EXPR_AND_VAL_AS_STRING(spans) << std::endl;

      double lambda = 10;

      Corrector<double> corr;
      auto pnavs = toArray(sail::map(spans, [&](Spani span) {
        std::cout << "Processing span " << span << std::endl;
        return getPolarNavData(corr, FilteredNavData(navs.slice(span.minv(), span.maxv()), lambda, FilteredNavData::NONE));
      }));
      auto data2 = computeStabilities(concat(pnavs));
      saveRawArray(cacheFilename, data2);
      return data2;
    } else {
      std::cout << "Successfully loaded cached target speed point data.\n";
      std::cout << "Keep in mind that it might be outdated if you changed the algorithm." << std::endl;
      return data;
    }
  }




  TargetSpeedSolver::Settings getSolverSettings(
      Settings settings,
      Array<TargetSpeedPoint> pts) {
    ScopedLog::setDepthLimit(3);

    constexpr bool autoTune = false;
    if (autoTune) { // <-- unsure how effective this strategy really is...
      return TargetSpeedSolver::optimizeParameters(settings.param,
          pts, TargetSpeedSolver::TuneSettings(), settings.solverSettings);
    } else {
      return settings.solverSettings;
    }
  }

  void processOpt(Settings s, Array<TargetSpeedPoint> pts) {
    auto results = TargetSpeedSolver::optimize(s.param, pts, getSolverSettings(s, pts));

    std::ofstream file(s.prefix + "_tgtspeed.txt");
    std::cout << EXPR_AND_VAL_AS_STRING(results.dataCost) << std::endl;
    results.targetSpeed.outputNorthSailsTable(&file);
    results.targetSpeed.plotPolarCurves(true);
    results.targetSpeed.plotPolarCurves(false);

    if (false) {
      auto curves = results.targetSpeed.samplePolarCurves(true);
      GnuplotExtra plot;
      plot.set_style("lines");
      for (int i = 0; i < curves.size(); i++) {
        plot.plot(curves[i]);
      }
      plot.set_style("points");
      plot.plot(makePlotData(pts));
      plot.show();
    }
  }


  void process(Settings settings, NavLoader loader) {
    std::cout << "Processing data... Intermediate data will \n";
    std::cout << "be saved to files beginning with " << settings.prefix << "\n";
    auto cnavs = getPolarNavDataFromRawNavs(
        settings.prefix + "_cachedpts.dat", loader);
    processOpt(settings, cnavs);
  }


  void processFolder(Settings s, std::string path) {
    Poco::Path p(path);
    s.prefix = "/tmp/tgtspeed_" + p.getBaseName();
    process(s, [&]() {
        return scanNmeaFolder(
            path, "unnamed");
    });
  }
}



int main(int argc, const char **argv) {
  std::string path = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("psaros33_Banque_Sturdza").get().toString();

  ArgMap amap;
  Settings s;
  amap.registerOption("--radial", "Set the radial regularization")
      .store(&s.solverSettings.radialReg);
  amap.registerOption("--iters", "Number of iterations in the solver")
      .store(&s.solverSettings.iters);
  amap.registerOption("--angular", "Set the angular regularization")
      .store(&s.solverSettings.angularReg);
  amap.registerOption("--path", "Provide a path to a folder with a dataset")
    .store(&path);

  switch (amap.parse(argc, argv)) {
    case ArgMap::Continue:
      processFolder(s, path);
      return 0;
    case ArgMap::Error:
      return -1;
    case ArgMap::Done:
      return 0;
  }
}


