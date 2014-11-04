/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/common/ArgMap.h>
#include <server/nautical/CalibratedNavData.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/TemporalSplit.h>
#include <server/common/ScopedLog.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>
#include <server/common/DataSplits.h>
#include <server/common/MeanAndVar.h>
#include <server/plot/extra.h>

using namespace sail;

namespace {
  std::string chunkSplitString(std::default_random_engine &e) {
    int len = 30;
    Arrayb x = makeChunkSplit(len, e);
    std::string s(len, ' ');
    for (int i = 0; i < len; i++) {
      s[i] = (x[i]? '#' : '-');
    }
    return s;
  }

  void chunkSplitDemo(std::default_random_engine &e) {
    for (int i = 0; i < 30; i++) {
      std::cout << chunkSplitString(e) << std::endl;
    }
  }


  MeanAndVar getMagHdg(CorrectorSet<double> &s, Array<Arrayd> params,
    Angle<double> a) {
    return MeanAndVar(
        params.map<double>([&](Arrayd p) {
        return s.magneticHeadingCorrector().correct(s.magneticHeadingParams(p.ptr()),
            a).degrees();
      })
    );
  }

  MeanAndVar getAwa(CorrectorSet<double> &s, Array<Arrayd> params,
    Angle<double> a) {
    return MeanAndVar(
        params.map<double>([&](Arrayd p) {
        return s.awaCorrector().correct(s.awaParams(p.ptr()),
            a).degrees();
      })
    );
  }

  MeanAndVar getAws(CorrectorSet<double> &s, Array<Arrayd> params,
    Velocity<double> a) {
    return MeanAndVar(
        params.map<double>([&](Arrayd p) {
        return s.awsCorrector().correct(s.awsParams(p.ptr()),
            a).knots();
      })
    );
  }

  MeanAndVar getWatSpeed(CorrectorSet<double> &s, Array<Arrayd> params,
    Velocity<double> a) {
    return MeanAndVar(
        params.map<double>([&](Arrayd p) {
        return s.waterSpeedCorrector().correct(s.waterSpeedParams(p.ptr()),
            a).knots();
      })
    );
  }

  void calibrationReport(Array<Arrayd> values) {
    std::cout << "OPTIMAL PARAMETER VECTORS:" << std::endl;
    for (int i = 0; i < values.size(); i++) {
      std::cout << EXPR_AND_VAL_AS_STRING(values[i]) << std::endl;
    }
    Angle<double> a = Angle<double>::degrees(0);
    DefaultCorrectorSet<double> s;
    std::cout << "A magnetic angle of 0 degs maps to " <<
        getMagHdg(s, values, a) << " degrees" << std::endl;
    std::cout << "An AWA angle of 0 degs maps to " <<
        getAwa(s, values, a) << " degrees" << std::endl;
    int count = 12;
    LineKM line(0, count-1, log(1.0), log(40.0));
    for (int i = 0; i < count; i++) {
      Velocity<double> vel = Velocity<double>::knots(exp(line(i)));
      std::cout << "An AWS of " << vel.knots() << " knots maps to " <<
          getAws(s, values, vel) << " knots" << std::endl;
    }
    for (int i = 0; i < count; i++) {
      Velocity<double> vel = Velocity<double>::knots(exp(line(i)));
      std::cout << "A wat speed of " << vel.knots() << " knots maps to " <<
      getWatSpeed(s, values, vel) << " knots" << std::endl;
    }
  }

  void ex0(double lambda, CalibratedNavData::Settings settings) {
    ENTERSCOPE("Running a preconfigured example");
    SCOPEDMESSAGE(INFO, "Loading psaros33 data");
    Array<Nav> navs = scanNmeaFolder(PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets/psaros33_Banque_Sturdza").get(),
      Nav::debuggingBoatId());
    Array<Spani> spans = recursiveTemporalSplit(navs);
    Spani span = spans[5];
    SCOPEDMESSAGE(INFO, "Filtering the data...");
    FilteredNavData fdata(navs.slice(span.minv(), span.maxv()), lambda);
    SCOPEDMESSAGE(INFO, "Calibrating...");
    Arrayd times = fdata.makeCenteredX();
    int middle = times.size()/2;

    CorrectorSet<adouble>::Ptr corr;
    CalibratedNavData calibA = CalibratedNavData(fdata, times.sliceTo(middle), corr, settings);
    CalibratedNavData calibB = CalibratedNavData(fdata, times.sliceFrom(middle), corr, settings);
    Array<Arrayd> params = Array<Arrayd>::args(calibA.optimalCalibrationParameters(),
                                               calibB.optimalCalibrationParameters());
    SCOPEDMESSAGE(INFO, "Done calibrating.");
    calibrationReport(params);
  }

  void extrema(double lambda, int count) {
    ENTERSCOPE("Running a preconfigured example");
    SCOPEDMESSAGE(INFO, "Loading psaros33 data");
    Array<Nav> navs = scanNmeaFolder(PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets/psaros33_Banque_Sturdza").get(),
      Nav::debuggingBoatId());
    Array<Spani> spans = recursiveTemporalSplit(navs);
    Spani span = spans[5];
    SCOPEDMESSAGE(INFO, "Filtering the data...");
    FilteredNavData fdata(navs.slice(span.minv(), span.maxv()), lambda);
    Arrayd times = fdata.makeCenteredX();
    fdata.magHdg().interpolateLinear(times);
  }

  void cmp0(CalibratedNavData::Settings settings, int splitCount, std::default_random_engine &e, double lambda, Angle<double> corruptAwa, Angle<double> corruptMagHdg) {
    ENTERSCOPE("Compare several methods");
     SCOPEDMESSAGE(INFO, "Loading psaros33 data");
     Array<Nav> navs = scanNmeaFolder(PathBuilder::makeDirectory(Env::SOURCE_DIR)
       .pushDirectory("datasets/psaros33_Banque_Sturdza").get(),
       Nav::debuggingBoatId());

     Array<Spani> spans = recursiveTemporalSplit(navs);
     Spani span = spans[5];
     SCOPEDMESSAGE(INFO, "Filtering the data...");
     FilteredNavData fdata(navs.slice(span.minv(), span.maxv()), lambda);
     fdata.setAwa(fdata.awa() - corruptAwa);
     fdata.setMagHdg(fdata.magHdg() - corruptMagHdg);
     SCOPEDMESSAGE(INFO, "Calibrating...");

     Arrayd times = fdata.makeCenteredX();

     Array<Arrayb> splits;
     if (splitCount == 1) {
       splits = Array<Arrayb>::args(Arrayb::fill(times.size(), true));
     } else {
       splits = makeChunkSplits(splitCount, times.size(), e, 0.7);
     }


     int settingCounter = 0;
     int settingCount = CalibratedNavData::Settings::COST_TYPE_COUNT*CalibratedNavData::Settings::WEIGHT_TYPE_COUNT;
     assert(settingCount == 6);
     int totalCount = splitCount*settingCount;
     int totalCounter = 0;
     assert(totalCount >= 0);
     Array<Array<CalibratedNavData> > resultsPerSetting(settingCount);
     for (int i = 0; i < CalibratedNavData::Settings::COST_TYPE_COUNT; i++) {
       settings.costType = CalibratedNavData::Settings::CostType(i);
       for (int j = 0; j < CalibratedNavData::Settings::WEIGHT_TYPE_COUNT; j++) {
         settings.weightType = CalibratedNavData::Settings::WeightType(j);
         Array<CalibratedNavData> results(splitCount);
         for (int k = 0; k < splitCount; k++) {
           ENTERSCOPE(stringFormat("CMP1 ITERATION %d/%d", totalCounter+1, totalCount));
           results[k] = CalibratedNavData(fdata, times.slice(splits[k]),
               CorrectorSet<adouble>::Ptr(), settings);


           totalCounter++;
         }
         resultsPerSetting[settingCounter] = results;




         settingCounter++;
       }
     }
     assert(settingCount == settingCounter);

     for (int i = 0; i < settingCount; i++) {
       resultsPerSetting[i][0].outputGeneralInfo(&std::cout);
       Array<Arrayd> params = resultsPerSetting[i].map<Arrayd>([&](CalibratedNavData x) {
         return x.optimalCalibrationParameters();
       });
       calibrationReport(params);
     }
     std::cout << "AWA corruption (should be close to AWA offset): "
         << corruptAwa.degrees() << " degrees" << std::endl;
     std::cout << "Mag hdg corruption (should be close to mag hdg offset): "
         << corruptMagHdg.degrees() << " degrees" << std::endl;
     SCOPEDMESSAGE(INFO, "Done calibrating.");
  }


  void plot0(double lambda, CalibratedNavData::Settings settings) {
    ENTERSCOPE("Plot true wind and current");
    SCOPEDMESSAGE(INFO, "Loading psaros33 data");
    Array<Nav> navs = scanNmeaFolder(PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets/psaros33_Banque_Sturdza").get(),
      Nav::debuggingBoatId());
    Array<Spani> spans = recursiveTemporalSplit(navs);
    Spani span = spans[5];
    SCOPEDMESSAGE(INFO, "Filtering the data...");
    FilteredNavData fdata(navs.slice(span.minv(), span.maxv()), lambda);
    SCOPEDMESSAGE(INFO, "Calibrating...");
    Arrayd times = fdata.makeCenteredX();

    CalibratedNavData calib(fdata, times, CorrectorSet<adouble>::Ptr(), settings);
    int count = times.size();
    Arrayd allwindX(count), allwindY(count), allcurrentX(count), allcurrentY(count);
    for (int i = 0; i < count; i++) {
      CalibratedValues<double> c = calib.calibratedValues(times[i]);
      allwindX[i] = c.trueWind[0].knots();
      allwindY[i] = c.trueWind[1].knots();
      allcurrentX[i] = c.trueCurrent[0].knots();
      allcurrentY[i] = c.trueCurrent[1].knots();
    }

    double len = Duration<double>::minutes(60).seconds();
    int plotCount = int(ceil((times.last() - times.first())/len));
    LineKM endpts(0, plotCount, 0, times.size());
    for (int i = 0; i < plotCount; i++) {
      GnuplotExtra plot;
      plot.set_style("lines");
      plot.set_xlabel("Time (seconds)");
      plot.set_ylabel("Speed (knots)");
      plot.set_title(stringFormat("Plot %d/%d", i+1, plotCount));
      int from = int(endpts(i));
      int to = int(endpts(i+1));
      Arrayd timessub = times.slice(from, to);
      plot.plot_xy(timessub, allwindX.slice(from, to), "True wind X");
      plot.plot_xy(timessub, allwindY.slice(from, to), "True wind Y");
      plot.plot_xy(timessub, allcurrentX.slice(from, to), "True current X");
      plot.plot_xy(timessub, allcurrentY.slice(from, to), "True current Y");
      plot.show();
    }
  }


  CalibratedNavData::Settings::CostType mapStrToCostType(const std::string &x) {
    if (x == "l2") {
      return CalibratedNavData::Settings::L2_COST;
    } else if (x == "l1") {
      return CalibratedNavData::Settings::L1_COST;
    } else {
      LOG(FATAL) << stringFormat("Unable to map string %s to a cost type",
          x.c_str());
      return CalibratedNavData::Settings::COST_TYPE_COUNT;
    }
  }

  CalibratedNavData::Settings::WeightType mapStrToWeightType(const std::string &x) {
    if (x == "direct") {
      return CalibratedNavData::Settings::DIRECT;
    } else if (x == "sqrtabs") {
      return CalibratedNavData::Settings::SQRT_ABS;
    } else if (x == "uniform") {
      return CalibratedNavData::Settings::UNIFORM;
    } else {
      LOG(FATAL) << stringFormat("Unable to map string %s to a weight type", x.c_str());
      return CalibratedNavData::Settings::WEIGHT_TYPE_COUNT;
    }
  }
}

int main(int argc, const char **argv) {
  std::default_random_engine e;
  ArgMap amap;
  double lambda = 1000;
  int verbosity = 9;
  int sampleCount = 30000;
  double awaCorruption = 0;
  double magHdgCorruption = 0;
  int splitCount = 8;
  int extremaCount = 12;

  std::string costType = "l2";
  std::string weightType = "direct";

  CalibratedNavData::Settings settings;

  amap.registerOption("--corrupt-awa", "Subtract a [value] in degrees from all awa values, making it corrupt")
      .setArgCount(1).store(&awaCorruption);
  amap.registerOption("--corrupt-mag-hdg", "Subtract a [value] in degrees from all mag hdg values, making it corrupt")
      .setArgCount(1).store(&magHdgCorruption);
  amap.registerOption("--split-count", "Set the [number] of splits used for cross validation")
    .setArgCount(1).store(&splitCount);
  amap.registerOption("--ex0", "Run a preconfigured example");
  amap.registerOption("--cmp0", "Run a systematic comparison of different strategies using Objf1");
  amap.registerOption("--plot0", "Make a plot of true wind and current");
  amap.registerOption("--lambda", "Set the regularization parameter")
      .setArgCount(1).store(&lambda);
  amap.registerOption("--sample-count", "Set the number of equations used for calibration")
    .setArgCount(1).store(&sampleCount);
  amap.registerOption("--verbosity", "Set the verbosity")
    .setArgCount(1).store(&verbosity);
  amap.registerOption("--cost-type", "Provide 'l1' or 'l2' to set the cost type")
    .setArgCount(1).store(&costType);
  amap.registerOption("--weight-type", "Provide 'direct', 'sqrtabs' or 'uniform' to set the weight type")
      .setArgCount(1).store(&weightType);
  amap.registerOption("--order", "Set the order of the differentiation")
      .setArgCount(1).store(&settings.order);
  amap.registerOption("--extrema", "Detect extrema in the signal");

  if (!amap.parse(argc, argv)) {
    return -1;
  }
  ScopedLog::setDepthLimit(verbosity);
  settings.costType = mapStrToCostType(costType);
  settings.weightType = mapStrToWeightType(weightType);
  if (amap.helpAsked()) {
    return 0;
  }

  if (amap.optionProvided("--ex0")) {
    ex0(lambda, settings);
  } else if (amap.optionArgs("--extrema")) {
    extrema(lambda, extremaCount);
  } else if (amap.optionProvided("--cmp0")) {
    cmp0(settings, splitCount, e, lambda, Angle<double>::degrees(awaCorruption), Angle<double>::degrees(magHdgCorruption));
  } else if (amap.optionProvided("--plot0")) {
    plot0(lambda, settings);
  }

  return 0;
}
