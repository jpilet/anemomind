/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TargetSpeed.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>

using namespace sail;

namespace {
  void targetSpeedPlot() {
    Poco::Path srcpath = PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").
        pushDirectory("regates").get();
    Array<Nav> navs = scanNmeaFolder(srcpath, Nav::debuggingBoatId());
    Arrayb upwind = guessUpwindNavs(navs);
    Arrayb downwind = neg(upwind);
  }
}

int main() {
  targetSpeedPlot();
  return 0;
}
