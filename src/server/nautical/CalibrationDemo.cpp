/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/common/ArgMap.h>
#include <server/nautical/CalibratedNavData.h>
#include <server/nautical/NavNmeaScan.h>

using namespace sail;

int main(int argc, const char **argv) {
  ArgMap amap;
  amap.registerOption("--ex0", "Run a preconfigured example");

  if (!amap.parse(argc, argv)) {
    return -1;
  }
  if (amap.help()) {
    return 0;
  }



  return 0;
}
