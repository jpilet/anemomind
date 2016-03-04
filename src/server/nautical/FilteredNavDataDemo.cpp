/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/logs/LogLoader.h>
#include <server/common/Env.h>
#include <server/nautical/TemporalSplit.h>
#include <server/math/ContinuousAngles.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/common/math.h>
#include <server/math/CleanNumArray.h>
#include <server/math/nonlinear/GeneralizedTV.h>
#include <server/nautical/FilteredNavData.h>
#include <server/common/PhysicalQuantityIO.h>


using namespace sail;
using namespace sail::NavCompat;

namespace {


  void dispFilteredNavData(NavDataset navs, Spani span, int modeType, double lambda) {
    auto selected = sliceTo(slice(navs, span.minv(), span.maxv()), 30*60);
    FilteredNavData data(selected,
        lambda, (modeType == 2? FilteredNavData::DERIVATIVE : FilteredNavData::SIGNAL));
    FilteredNavData::NoiseStdDev stddev = data.estimateNoise(selected);
    std::cout << EXPR_AND_VAL_AS_STRING(stddev.awa) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(stddev.aws) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(stddev.gpsBearing) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(stddev.gpsSpeed) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(stddev.magHdg) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(stddev.watSpeed) << std::endl;
  }

  void ex0(int mode, double lambda) {

    auto navs = LogLoader::loadNavDataset(
        std::string(Env::SOURCE_DIR) + "/datasets/psaros33_Banque_Sturdza");
      Array<Spani> spans = recursiveTemporalSplit(navs);
    dispFilteredNavData(navs, spans[5], mode, lambda);
  }
}

int main(int argc, const char **argv) {
  int mode = 1;
  double lambda = 10;
  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--plotmode",
      "What to display: mode=1 displays the signals, mode=2 displays the derivatives")
      .setArgCount(1).store(&mode);
  amap.registerOption("--ex0", "Run a preconfigured example on the Psaros33 dataset");
  amap.registerOption("--overview", "Segment the loaded navs into chunks and display those chunks");
  amap.registerOption("--lambda", "Set the regularization parameter").setArgCount(1).store(&lambda);
  if (amap.parse(argc, argv) == ArgMap::Error) {
    return -1;
  }

  if (!amap.helpAsked()) {
    if (amap.optionProvided("--ex0")) {
      ex0(mode, lambda);
    } else {
      NavDataset navs = getTestdataNavs(amap);
      Array<Spani> spans = recursiveTemporalSplit(navs);
      if (amap.optionProvided("--overview")) {
        dispTemporalRaceOverview(spans, navs);
      } else {
        dispFilteredNavData(navs, Spani(0, getNavSize(navs)), mode, lambda);
      }
    }
  }
  return 0;
}
