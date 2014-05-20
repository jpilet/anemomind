/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TargetSpeed.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Span.h>
#include <iostream>
#include <server/plot/extra.h>

using namespace sail;

namespace {
  void targetSpeedPlot() {
    Poco::Path srcpath = PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").
        pushDirectory("regates").get();
    Array<Nav> allnavs = scanNmeaFolder(srcpath, Nav::debuggingBoatId());
    Arrayb upwind = guessUpwindNavsByTwa(allnavs);
    Array<Nav> upwindNavs = allnavs.slice(upwind);

    const int binCount = 25;
    Array<Velocity<double> > tws = estimateRawTws(upwindNavs);
    Array<Velocity<double> > vmg = calcUpwindVmg(upwindNavs);
    Array<Velocity<double> > gss = gpsSpeed(upwindNavs);

    Arrayd gssd = gss.map<double>([&](Velocity<double> x) {return x.metersPerSecond();});
    Arrayd twsd = tws.map<double>([&](Velocity<double> x) {return x.metersPerSecond();});
    Arrayd vmgd = vmg.map<double>([&](Velocity<double> x) {return x.metersPerSecond();});

    std::cout << "GPS-span (m/s): " << Spand(gssd) << std::endl;
    std::cout << "TWS-span (m/s): " << Spand(twsd) << std::endl;
    std::cout << "VMG-span (m/s): " << Spand(vmgd) << std::endl;

    Velocity<double> minvel = Velocity<double>::metersPerSecond(4.0);
    Velocity<double> maxvel = Velocity<double>::metersPerSecond(17.0);
    TargetSpeedData tgt(tws, vmg, HistogramMap(25,
        minvel.metersPerSecond(), maxvel.metersPerSecond()));

    const bool dispHist = false;
    if (dispHist) {
      const HistogramMap &hist = tgt.hist();
      GnuplotExtra plot;
      plot.set_style("lines");
      //plot.plot(hist.makePlotData(hist.countPerBin(twsd)));
      //plot.plot(hist.makePlotData(hist.countPerBin(vmgd)));
      plot.plot(hist.makePlotData(hist.countPerBin(gssd)));
      plot.show();
    }



    tgt.plot();
  }
}

int main() {
  targetSpeedPlot();
  return 0;
}
