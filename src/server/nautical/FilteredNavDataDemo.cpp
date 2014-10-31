/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/nautical/TemporalSplit.h>
#include <server/math/ContinuousAngles.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/common/math.h>
#include <server/math/CleanNumArray.h>
#include <server/math/nonlinear/GeneralizedTV.h>
#include <server/nautical/FilteredNavData.h>


using namespace sail;

namespace {


  void dispFilteredNavData(Array<Nav> navs, Spani span, int modeType, double lambda) {
    FilteredNavData data(navs.slice(span.minv(), span.maxv()),
        lambda, (modeType == 2? FilteredNavData::DERIVATIVE : FilteredNavData::SIGNAL));
  }

  void ex0(int mode, double lambda) {
    Array<Nav> navs =
        scanNmeaFolder(std::string(Env::SOURCE_DIR) + "/datasets/psaros33_Banque_Sturdza",
        Nav::debuggingBoatId());
    Array<Spani> spans = recursiveTemporalSplit(navs);
    dispFilteredNavData(navs, spans[5], mode, lambda);
  }
}

int main(int argc, const char **argv) {
  int mode = 1;
  double lambda = 3000;
  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--plotmode",
      "What to display: mode=1 displays the signals, mode=2 displays the derivatives")
      .setArgCount(1).store(&mode);
  amap.registerOption("--ex0", "Run a preconfigured example on the Psaros33 dataset");
  amap.registerOption("--overview", "Segment the loaded navs into chunks and display those chunks");
  amap.registerOption("--lambda", "Set the regularization parameter").setArgCount(1).store(&lambda);
  if (!amap.parseAndHelp(argc, argv)) {
    return -1;
  }

  if (!amap.helpAsked()) {
    if (amap.optionProvided("--ex0")) {
      ex0(mode, lambda);
    } else {
      Array<Nav> navs = getTestdataNavs(amap);
      Array<Spani> spans = recursiveTemporalSplit(navs);
      if (amap.optionProvided("--overview")) {
        dispTemporalRaceOverview(spans, navs);
      } else {
        dispFilteredNavData(navs, Spani(0, navs.size()), mode, lambda);
      }
    }
  }
  return 0;
}
