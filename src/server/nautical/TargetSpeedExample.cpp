/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TargetSpeed.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmeaScan.h>
#include <iostream>
#include <server/nautical/Calibrator.h>
#include <server/nautical/BoatLogProcessor.h>

using namespace sail;

namespace {
  void targetSpeedPlot() {
    Poco::Path srcpath = PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").
        pushDirectory("Irene").get();
    Array<Nav> allnavs = scanNmeaFolder(srcpath, Nav::debuggingBoatId());
    assert(!allnavs.empty());

    Calibrator c;
    c.calibrate(allnavs);
    c.print();


    TargetSpeedData uw, dw;
    computeTargetSpeedData(c, &uw, &dw);

    TargetSpeedData which = uw;
    which.plot();
  }
}

int main() {
  targetSpeedPlot();
  return 0;
}
