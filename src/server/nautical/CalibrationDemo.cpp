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

using namespace sail;

namespace {
  void ex0(double lambda) {
    Array<Nav> navs = scanNmeaFolder(PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets/psaros33_Banque_Sturdza").get(),
      Nav::debuggingBoatId());
    Array<Spani> spans = recursiveTemporalSplit(navs);
    Spani span = spans[5];
    FilteredNavData fdata(navs, lambda);
    CalibratedNavData calib(fdata);
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  double lambda = 1000;
  amap.registerOption("--ex0", "Run a preconfigured example");
  amap.registerOption("--lambda", "Set the regularization parameter")
      .setArgCount(1).store(&lambda);

  if (!amap.parse(argc, argv)) {
    return -1;
  }
  if (amap.help()) {
    return 0;
  }

  if (amap.optionProvided("--ex0")) {
    ex0(lambda);
  } else {

  }

  return 0;
}
