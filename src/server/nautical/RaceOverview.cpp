/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/TestdataNavs.h>
#include <server/nautical/TemporalSplit.h>

using namespace sail;

namespace {
  void dispSpans(Array<Spani> spans, Array<Nav> navs) {
    int spanCount = spans.size();
    for (auto span : spans) {
      Array<Nav> sub = navs.slice(span.minv(), span.maxv());
      std::cout << "[" << span.minv() << ", " << span.maxv() << "[" << std::endl;
      std::cout << "   from     " << sub.first().time().toString() << std::endl;
      std::cout << "   duration " << (sub.last().time() - sub.first().time()).str() << std::endl;
    }
    std::cout << "Total of " << spanCount << " race episodes." << std::endl;
  }
}


int main(int argc, const char **argv) {
  double lowerThresh = 8;
  double relativeThresh = 0.1;

  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--lower-thresh", "Set the lower threshold in seconds")
      .setArgCount(1).store(&lowerThresh);
  amap.registerOption("--rel-thresh", "Set the relative time threshold")
      .setArgCount(1).store(&relativeThresh);

  if (amap.parseAndHelp(argc, argv)) {
    Array<Nav> navs = getTestdataNavs(amap);
    Array<Spani> spans = recursiveTemporalSplit(navs,
        relativeThresh, Duration<double>::seconds(lowerThresh));
    dispSpans(spans, navs);
    return 0;
  }
  return -1;
}


