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
#include <server/plot/extra.h>

using namespace sail;

void plotNavAwa(Array<Nav> navs) {
  GnuplotExtra plot;

  int count = navs.size();
  MDArray2d XY(count, 2);
  int counter = 0;
  for (int i = 0; i < count; i++) {
    double awa = navs[i].awa().radians();
    if (!std::isnan(awa)) {
      XY(i, 0) = cos(awa);
      XY(i, 1) = sin(awa);
      counter++;
    }
  }
  plot.plot(XY.sliceRowsTo(counter));

  plot.show();
}

namespace {
  void targetSpeedPlot() {
    Poco::Path srcpath = PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").
        pushDirectory("Irene").get();
    Array<Nav> allnavs = scanNmeaFolder(srcpath, Nav::debuggingBoatId());
    assert(!allnavs.empty());

    plotNavAwa(allnavs);


    Calibrator c;
    //c.calibrate(allnavs);
    Calibrator::WindEstimator::initializeParameters(c.calibrationValues());
    c.print();




    TargetSpeedData uw, dw;
    computeTargetSpeedData(c, allnavs, &uw, &dw);
    uw.plot();
//    int choice = 0;
//    do {
//      std::cout << "What do you want to do?\n"
//          " 0) quit\n"
//          " 1) Plot upwind target speed\n"
//          " 2) Plot downwind target speed\n" << std::endl;
//      std::cin >> choice;
//      if (choice == 1) {
//        uw.plot();
//      }
//      if (choice == 2) {
//        dw.plot();
//      }
//    } while (choice != 0);
  }
}

int main() {
  targetSpeedPlot();
  return 0;
}
