/*
 *  Created on: 2014-09-04
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/TestdataNavs.h>

using namespace sail;

int main(int argc, const char **argv) {
  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--slice2d", "For a given windspeed (first argument), "
      "plot the N (second argument) closest point to that wind speed.").setArgCount(2).setUnique();
  amap.setHelpInfo("Produces a scatter polar plot, no calibration.");
  if (amap.parseAndHelp(argc, argv)) {
    return 0;
  }
  return -1;
}


