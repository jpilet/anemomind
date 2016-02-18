/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/TestdataNavs.h>
#include <server/nautical/TemporalSplit.h>
#include <iostream>

using namespace sail;

int main(int argc, const char **argv) {
  double lowerThresh = 8;
  double relativeThresh = 0.1;

  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--lower-thresh", "Set the lower threshold in seconds")
      .setArgCount(1).store(&lowerThresh);
  amap.registerOption("--rel-thresh", "Set the relative time threshold")
      .setArgCount(1).store(&relativeThresh);

  if (amap.parse(argc, argv) != ArgMap::Error) {
    NavCollection navs = getTestdataNavs(amap);
    Array<Spani> spans = recursiveTemporalSplit(navs,
        relativeThresh, Duration<double>::seconds(lowerThresh));
    dispTemporalRaceOverview(spans, navs);
    return 0;
  }
  return -1;
}


