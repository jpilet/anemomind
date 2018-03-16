/*
 * PolarDemo.cpp
 *
 *  Created on: 16 Mar 2018
 *      Author: jonas
 */

#include <server/nautical/logimport/LogLoader.h>
#include <server/common/logging.h>
#include <server/nautical/tgtspeed/RealPerfSurf.h>
#include <server/nautical/NavDataset.h>

using namespace sail;

int main(int argc, const char** argv) {
  auto path = "/Users/jonas/prog/anemomind_local/yquem2_simulated.log";
  auto ds = LogLoader::loadNavDataset(path);
  ds.preferSource(
      {TWS, TWDIR, TWA, VMG},
      "Simulated Anemomind estimator");
  ds.preferSource(
      {GPS_SPEED, GPS_BEARING},
      "mix (,  reparsed, Internal GPS) merged+filtered");

  LOG(INFO) << "Loaded" << path;
  auto results = optimizeRealPerfSurf(ds);
  LOG(INFO) << "Optimized" << path;
  LOG(INFO) << "Final sample count: " << results.finalSampleCount;
  return 0;
}
