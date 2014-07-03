/*
 *  Created on: 2014-07-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/WaterCalib.h>
#include <server/nautical/HorizontalMotionParam.h>
#include <iostream>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>
#include <server/common/ScopedLog.h>

using namespace sail;

int main(int argc, const char **argv) {
  ScopedLog::setDepthLimit(2);
  Array<Nav> navs = getTestdataNavs().sliceTo(5000);

  ZeroHorizontalMotionParam mparam;
  WaterCalib calib(mparam);
  WaterCalib::Results results = calib.optimizeRandomInits(navs, 30);
  //WaterCalib::Results results = calib.optimize(navs);
  std::cout << EXPR_AND_VAL_AS_STRING(results.params) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(results.inlierCount()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(results.inliers.size()) << std::endl;
  calib.makeWatSpeedCalibPlot(results.params, results.navs.slice(results.inliers));

  return 0;
}


