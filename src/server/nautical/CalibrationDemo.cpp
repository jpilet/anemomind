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

using namespace sail;

namespace {
  void ex0(double lambda) {
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
    Arrayd times = CalibratedNavData::sampleTimes(fdata, 1000);
    CalibratedNavData calib(fdata, times);
    SCOPEDMESSAGE(INFO, "Done calibrating.");
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  double lambda = 1000;
  int verbosity = 9;
  amap.registerOption("--ex0", "Run a preconfigured example");
  amap.registerOption("--lambda", "Set the regularization parameter")
      .setArgCount(1).store(&lambda);
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
    ex0(lambda);
  } else {

  }

  return 0;
}
