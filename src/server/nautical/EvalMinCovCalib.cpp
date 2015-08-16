/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */


#include <server/nautical/CalibrationBenchmark.h>
#include <server/nautical/MinCovCalib.h>

using namespace sail;

Corrector<double> performCalibration(Array<Nav> navs) {
  FilteredNavData data(navs, 0.5);
  return MinCovCalib::optimizeWindVsCurrent(data, MinCovCalib::Settings());
}

int main(int argc, const char **argv) {
  std::cout << reducedBenchmark(performCalibration);
  return 0;
}



