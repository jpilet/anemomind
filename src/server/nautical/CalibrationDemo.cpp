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

  void cmp0(std::default_random_engine &e, double lambda) {
    ENTERSCOPE("Compare several methods");
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

     int splitCount = 8;

     Array<Arrayb> splits = makeChunkSplits(splitCount, times.size(), e, 0.7);

     int settingCounter = 0;
     int settingCount = CalibratedNavData::Settings::COST_TYPE_COUNT*CalibratedNavData::Settings::WEIGHT_TYPE_COUNT;
     assert(settingCount == 6);
     int totalCount = splitCount*settingCount;
     int totalCounter = 0;
     assert(totalCount == 48);
     CalibratedNavData::Settings settings;
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

     SCOPEDMESSAGE(INFO, "Done calibrating.");
  }


  void plot0(double lambda) {
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
    int middle = times.size()/2;


    CalibratedNavData calib(fdata, times);


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

  std::string costType = "l2";
  std::string weightType = "direct";

  CalibratedNavData::Settings settings;

  amap.registerOption("--ex0", "Run a preconfigured example");
  amap.registerOption("--ex0-cmp-1", "Run a systematic comparison of different strategies using Objf1");
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

  if (!amap.parse(argc, argv)) {
    return -1;
  }
  ScopedLog::setDepthLimit(verbosity);
  settings.costType = mapStrToCostType(costType);
  settings.weightType = mapStrToWeightType(weightType);
  if (amap.help()) {
    return 0;
  }

  if (amap.optionProvided("--ex0")) {
    ex0(lambda, settings);
  } else if (amap.optionProvided("--cmp0")) {
    cmp0(e, lambda);
  } else if (amap.optionProvided("--plot0")) {

  }

  return 0;
}
