/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BoatLogProcessor.h"


#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>
#include <fstream>
#include <iostream>
#include <Poco/File.h>
#include <Poco/JSON/Stringifier.h>
#include <server/common/ArgMap.h>
#include <server/common/Env.h>
#include <server/common/HierarchyJson.h>
#include <server/common/Json.h>
#include <server/common/PathBuilder.h>
#include <server/common/ScopedLog.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/nautical/calib/Calibrator.h>
#include <server/nautical/DownsampleGps.h>
#include <server/nautical/HTreeJson.h>
#include <server/nautical/NavJson.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/TargetSpeed.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/plot/extra.h>

#include <server/common/Json.impl.h> // This one should probably be the last one.

namespace sail {

using namespace NavCompat;

namespace {

using namespace sail;
using namespace std;

namespace {
  TargetSpeed makeTargetSpeedTable(bool isUpwind,
      std::shared_ptr<HTree> tree, Array<HNode> nodeinfo,
      const NavDataset& allnavs,
      std::string description) {

    // TODO: How to best select upwind/downwind navs? Is the grammar reliable for this?
    //   Maybe replace AWA by TWA in order to label states in Grammar001.
    Array<std::pair<TimeStamp, TimeStamp>> sel =
      markNavsByDesc(tree, nodeinfo, allnavs, description);

    ArrayBuilder<Velocity<double>> twsArray;
    ArrayBuilder<Velocity<double>> vmgArray;

    for (const std::pair<TimeStamp, TimeStamp>& it : sel) {
      NavDataset leg = allnavs.slice(it.first, it.second);

      TimedSampleRange<Velocity<double>> twsLeg = leg.samples<TWS>();
      TimedSampleRange<Velocity<double>> vmgLeg = leg.samples<VMG>();

      for (TimeStamp time(it.first); time < it.second; time += Duration<>::seconds(1)) {
        Optional<TimedValue<Velocity<>>> tws = twsLeg.evaluate(time);
        Optional<TimedValue<Velocity<>>> vmg = vmgLeg.evaluate(time);
        if (tws.defined() && vmg.defined()) {
          twsArray.add(tws.get().value);
          vmgArray.add(vmg.get().value);
        }
      }
    }

    // TODO: Adapt these values to the amount of recorded data.
    const int binCount = TargetSpeedTable::NUM_ENTRIES;
    Velocity<double> minvel = Velocity<double>::knots(0);
    Velocity<double> maxvel = Velocity<double>::knots(TargetSpeedTable::NUM_ENTRIES-1);
    Array<Velocity<double> > bounds = makeBoundsFromBinCenters(TargetSpeedTable::NUM_ENTRIES, minvel, maxvel);
    return TargetSpeed(isUpwind, twsArray.get(), vmgArray.get(), bounds);
  }

  void outputTargetSpeedTable(
      bool debug,
      std::shared_ptr<HTree> tree,
      Array<HNode> nodeinfo,
      NavDataset navs,
      std::ofstream *file) {
    TargetSpeed uw = makeTargetSpeedTable(true, tree, nodeinfo, navs, "upwind-leg");
    TargetSpeed dw = makeTargetSpeedTable(false, tree, nodeinfo, navs, "downwind-leg");

    if (debug) {
      LOG(INFO) << "Upwind";
      uw.plot();
      LOG(INFO) << "Downwind";
      dw.plot();
    }

    saveTargetSpeedTableChunk(file, uw, dw);
  }

  void makeBoatDatFile(
      bool debug,
      NavDataset src,
      PathBuilder outdir,
      std::shared_ptr<HTree> fulltree, Nav::Id boatId, WindOrientedGrammar g) {
    ENTERSCOPE("Output boat.dat with target speed data.");
    std::ofstream boatDatFile(outdir.makeFile("boat.dat").get().toString());

    Calibrator calibrator(g);

    NavDataset simulated = src;
    if (calibrator.calibrate(src, fulltree, boatId)) {
      calibrator.saveCalibration(&boatDatFile);
      simulated = calibrator.simulate(src);
    }
    assert(getNavSize(simulated) == getNavSize(src));

    outputTargetSpeedTable(debug, fulltree, g.nodeInfo(), simulated, &boatDatFile);
  }

  bool processBoatData(bool debug, Nav::Id boatId, NavDataset navs, Poco::Path dstPath, std::string filenamePrefix) {
    ENTERSCOPE("processBoatData");
    if (getNavSize(navs) == 0) {
      LOG(ERROR) << "No data to process.";
      return false;
    }
    SCOPEDMESSAGE(INFO, stringFormat("Process %d navs ranging from %s to %s",
        getNavSize(navs), getFirst(navs).time().toString().c_str(),
        getLast(navs).time().toString().c_str()));

    SCOPEDMESSAGE(INFO, "Parse data...");
    WindOrientedGrammarSettings settings;
    WindOrientedGrammar g(settings);
    std::shared_ptr<HTree> fulltree = g.parse(navs);
    SCOPEDMESSAGE(INFO, "done.");

    Poco::File buildDir(dstPath);

    // It could be that the directory already exists, and in that case,
    // we are going to overwrite previous data.
    buildDir.createDirectory();

    PathBuilder outdir = PathBuilder::makeDirectory(dstPath);
    std::string prefix = "all";

    makeBoatDatFile(debug, navs, outdir, fulltree, boatId, g);

    return true;
  }
}

void dispTargetSpeedTable(const char *label, const TargetSpeedTable &table, FP8_8 *data) {
  MDArray2d x(table.NUM_ENTRIES, 2);
  for (int i = 0; i < table.NUM_ENTRIES; i++) {
    x(i, 0) = double(table.binCenter(i));
    x(i, 1) = double(data[i]);
  }
  GnuplotExtra plot;
  plot.set_title(label);
  plot.set_style("lines");
  plot.plot(x);
  plot.show();
}

// For debugging purposes
void visualizeBoatDat(Poco::Path dstPath) {
  auto boatDatPath = PathBuilder::makeDirectory(dstPath).makeFile("boat.dat").get().toString();
  LOG(INFO) << "Load from " << boatDatPath;
  TargetSpeedTable table;
  CHECK(loadTargetSpeedTable(boatDatPath.c_str(), &table));
  dispTargetSpeedTable("Downwind", table, table._downwind);
  dispTargetSpeedTable("Upwind", table, table._upwind);
}

Nav::Id extractBoatId(Poco::Path path) {
  return path.directory(path.depth()-1);
}

}  // namespace

NavDataset loadNavs(ArgMap &amap, std::string boatId) {
  LogLoader loader;
  for (auto dirNameObj: amap.optionArgs("--dir")) {
    loader.load(dirNameObj->value());
  }
  for (auto fileNameObj: amap.optionArgs("--file")) {
    loader.load(fileNameObj->value());
  }
  return loader.makeNavDataset();
}

Nav::Id getBoatId(ArgMap &amap) {
  if (amap.optionProvided("--boatid")) {
    return amap.optionArgs("--boatid")[0]->value();
  } else {
    return Nav::debuggingBoatId();
  }
}

Poco::Path getDstPath(ArgMap &amap) {
  if (amap.optionProvided("--dst")) {
    return PathBuilder::makeDirectory(amap.optionArgs("--dst")[0]->value()).get();
  } else if (amap.optionProvided("--dir")) {
    return PathBuilder::makeDirectory(amap.optionArgs("--dir")[0]->value())
      .pushDirectory("processed").get();
  } else {
    return PathBuilder::makeDirectory("/tmp/processed").get();
  }
}

int continueProcessBoatLogs(ArgMap &amap) {
  ENTERSCOPE("Anemomind boat log processor");
  bool debug = amap.optionProvided("--debug");
  Nav::Id id = getBoatId(amap);
  auto navs = downSampleGpsTo1Hz(loadNavs(amap, id));
  auto dstPath = getDstPath(amap);
  std::stringstream msg;
  msg << "Process " << getNavSize(navs) << " navs with boatid=" <<
      id << " and save results to " << dstPath.toString();
  SCOPEDMESSAGE(INFO, msg.str());
  if (processBoatData(debug, id, navs, dstPath, "all")) {
    if (debug) {
      visualizeBoatDat(dstPath);
    }
    return 0;
  }
  return -1;
}

int mainProcessBoatLogs(int argc, const char **argv) {
  ArgMap amap;
  amap.setHelpInfo("Help for boat log processor");

  amap.registerOption("--debug", "Display debug information and visualization")
    .setArgCount(0);

  amap.registerOption("--boatid", "Id of the boat")
      .setArgCount(1);

  amap.registerOption("--file", "Files to process")
      .setMinArgCount(1)
      .alias("-f");


  amap.registerOption("--dir", "Directories to process")
    .setMinArgCount(1)
    .alias("-d");

  amap.registerOption("--dst", "Destination directory path")
    .setArgCount(1);

  amap.disableFreeArgs();

  auto status = amap.parse(argc, argv);
  switch (status) {
   case ArgMap::Error:
     return -1;
   case ArgMap::Done:
     return 0;
   case ArgMap::Continue:
     return continueProcessBoatLogs(amap);
   default:
     LOG(FATAL) << "Unhandled ParseStatus code";
     return -1;
  };
}

bool processBoatDataFullFolder(bool debug, Poco::Path dataPath) {
  auto boatId = Nav::debuggingBoatId();
  return processBoatData(debug, boatId, LogLoader::loadNavDataset(dataPath),
      PathBuilder(dataPath).pushDirectory("processed").get(), "all");
}











} /* namespace sail */
