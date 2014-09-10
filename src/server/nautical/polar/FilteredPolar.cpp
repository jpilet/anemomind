/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/plot/extra.h>
#include <server/nautical/TestdataNavs.h>


using namespace sail;

namespace {


}

int main(int argc, const char **argv) {
  double lambda = 0.0;
  double stepSizeKnots = 0.0;

  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--reg", "Set the regularization parameter").setArgCount(1).store(&lambda);
  amap.registerOption("--step", "Set the step size in the quantization").setArgCount(1).store(&stepSizeKnots);

  if (amap.parseAndHelp(argc, argv)) {
    Array<Nav> navs = getTestdataNavs(amap);

    //MDArray2i inds = quantFilterVel(Velocity<double>::knots(stepSizeKnots), );

    return 0;
  }
  return -1;
}
