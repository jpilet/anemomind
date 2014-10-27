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

  void ex0(double lambda, int sampleCount) {
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
    CalibratedNavData calibA = CalibratedNavData::bestOfInits(9, fdata, times.sliceTo(middle));
    CalibratedNavData calibB = CalibratedNavData::bestOfInits(9, fdata, times.sliceFrom(middle));
    Array<Arrayd> params = Array<Arrayd>::args(calibA.optimalCalibrationParameters(),
                                               calibB.optimalCalibrationParameters());
    SCOPEDMESSAGE(INFO, "Done calibrating.");
    calibrationReport(params);
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  double lambda = 1000;
  int verbosity = 9;
  int sampleCount = 30000;
  amap.registerOption("--ex0", "Run a preconfigured example");
  amap.registerOption("--lambda", "Set the regularization parameter")
      .setArgCount(1).store(&lambda);
  amap.registerOption("--sample-count", "Set the number of equations used for calibration")
    .setArgCount(1).store(&sampleCount);
  amap.registerOption("--verbosity", "Set the verbosity")
    .setArgCount(1).store(&verbosity);

  if (!amap.parse(argc, argv)) {
    return -1;
  }
  ScopedLog::setDepthLimit(verbosity);
  if (amap.help()) {
    return 0;
  }

  if (amap.optionProvided("--ex0")) {
    ex0(lambda, sampleCount);
  } else {

  }

  return 0;
}
