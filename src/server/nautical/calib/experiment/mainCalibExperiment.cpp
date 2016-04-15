/*
 * experiment.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: jonas
 */

#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/NavCompatibility.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/nautical/calib/experiment/CalibExperiment.h>
#include <server/plot/extra.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>

using namespace sail;
using namespace sail::Experimental;

namespace {
  Array<Nav> getNavs() {
    LogLoader loader;

    auto base = PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets")
      .pushDirectory("boat5576e98ea14d91730cadeed7");

    loader.load(base.makeFile("00000000558E7B43.log").get());
    loader.load(base.makeFile("00000000558E7C6F.log").get());
    loader.load(base.makeFile("00000000558E7D9B.log").get());
    loader.load(base.makeFile("00000000558E7EC7.log").get());
    loader.load(base.makeFile("00000000558E7FF3.log").get());

    auto data = loader.makeNavDataset();
    Array<Nav> navs = NavCompat::makeArray(data);
    return navs;
  }

  Eigen::Vector4d readParams(int argc, const char **argv) {
    Eigen::Vector4d params = getDefaultParams();
    for (int i = 1; i < argc; i++) {
      params[i - 1] = std::stod(argv[i]);
    }
    return params;
  }
}


namespace {
  Eigen::Vector2d getB(const FlowMats &x) {
    return x.B;
  }
}

void testErrorRelation(int argc, const char **argv) {
  auto params = readParams(argc, argv);
  auto navs = getNavs();
  auto mats = makeCurrentMats(navs);
  auto flows = computeFlows(mats, params.data());

  int windowSize = 60;

  auto flowErrors = computeRelativeErrors(flows, windowSize);
  auto gpsErrors = computeRelativeErrors(sail::map(mats, &getB).toArray(),
      windowSize);

  GnuplotExtra plot;
    plot.set_xlabel("GPS cornerness");
    plot.set_ylabel("Flow cornerness");

    plot.plot_xy(gpsErrors, flowErrors);

    plot.show();

}

int main(int argc, const char **argv) {
  if (argc <= 1) {
    std::cout << "Proof-of-concept calibration metric based on 'cornerness' value, on Yquem data\n";
    std::cout << "Usage: Provide as arguments up to four "
        "floating point parameters. The default numbers are [1 0 0 0]";
  }
  testErrorRelation(argc, argv);
  return 0;
}


