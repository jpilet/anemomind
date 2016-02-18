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
#include <server/nautical/Calibrator.h>
#include <server/nautical/HTreeJson.h>
#include <server/nautical/NavJson.h>
#include <server/nautical/NavLoader.h>
#include <server/nautical/NavNmea.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/nautical/TargetSpeed.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/plot/extra.h>

#include <server/common/Json.impl.h> // This one should probably be the last one.

namespace sail {


namespace {

using namespace sail;
using namespace std;

namespace {
  TargetSpeed makeTargetSpeedTable(bool isUpwind,
      std::shared_ptr<HTree> tree, Array<HNode> nodeinfo,
      Array<Nav> allnavs,
      std::string description) {

    // TODO: How to best select upwind/downwind navs? Is the grammar reliable for this?
    //   Maybe replace AWA by TWA in order to label states in Grammar001.
    Arrayb sel = markNavsByDesc(tree, nodeinfo, allnavs, description);

    NavCollection navs = NavCollection::fromNavs(allnavs.slice(sel));

    Array<Velocity<double> > tws = estimateExternalTws(navs);
    Array<Velocity<double> > vmg = calcExternalVmg(navs, isUpwind);
    Array<Velocity<double> > gss = getGpsSpeed(allnavs);

    // TODO: Adapt these values to the amount of recorded data.
    const int binCount = TargetSpeedTable::NUM_ENTRIES;
    Velocity<double> minvel = Velocity<double>::knots(0);
    Velocity<double> maxvel = Velocity<double>::knots(TargetSpeedTable::NUM_ENTRIES-1);
    Array<Velocity<double> > bounds = makeBoundsFromBinCenters(TargetSpeedTable::NUM_ENTRIES, minvel, maxvel);
    return TargetSpeed(isUpwind, tws, vmg, bounds);
  }

  void outputTargetSpeedTable(
      bool debug,
      std::shared_ptr<HTree> tree,
      Array<HNode> nodeinfo,
      Array<Nav> navs,
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
      NavCollection coll,
      PathBuilder outdir, Array<Nav> navs,
      std::shared_ptr<HTree> fulltree, Nav::Id boatId, WindOrientedGrammar g) {
    ENTERSCOPE("Output boat.dat with target speed data.");
    std::ofstream boatDatFile(outdir.makeFile("boat.dat").get().toString());

    Calibrator calibrator(g);
    if (calibrator.calibrate(coll, fulltree, boatId)) {
      calibrator.saveCalibration(&boatDatFile);
      calibrator.simulate(&navs);
    }

    outputTargetSpeedTable(debug, fulltree, g.nodeInfo(), navs, &boatDatFile);
  }

  bool processBoatData(bool debug, Nav::Id boatId, NavCollection navs, Poco::Path dstPath, std::string filenamePrefix) {
    ENTERSCOPE("processBoatData");
    if (navs.size() == 0) {
      LOG(ERROR) << "No data to process.";
      return false;
    }
    SCOPEDMESSAGE(INFO, stringFormat("Process %d navs ranging from %s to %s",
        navs.size(), navs.first().time().toString().c_str(),
        navs.last().time().toString().c_str()));

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

    auto navs0 = navs.makeArray();
    makeBoatDatFile(debug, navs, outdir, navs0, fulltree, boatId, g);

    {
      std::string path = outdir.makeFile(prefix + "_tree.js").get().toString();
      ENTERSCOPE("Output tree (" + path + ")");
      ofstream file(path);
      Poco::JSON::Stringifier::stringify(json::serializeMapped(fulltree, navs, g.nodeInfo()), file, 0, 0);
    }{
      std::string path = outdir.makeFile(prefix + "_navs.js").get().toString();
      ENTERSCOPE("Output navs");
      ofstream file(path);
      Poco::JSON::Stringifier::stringify(json::serialize(navs0), file, 0, 0);
    }{
      std::string path = outdir.makeFile(prefix + "_tree_node_info.js").get().toString();
      ENTERSCOPE("Output tree node info");
      ofstream file(path);
      Poco::JSON::Stringifier::stringify(json::serialize(g.nodeInfo()), file, 0, 0);
    }
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







}

NavCollection loadNavs(ArgMap &amap, std::string boatId) {
  ArrayBuilder<NavCollection > navs;
  for (auto dirNameObj: amap.optionArgs("--dir")) {
    navs.add(scanNmeaFolderWithSimulator(dirNameObj->value(), boatId));
  }
  for (auto fileNameObj: amap.optionArgs("--file")) {
    NavCollection navs = loadNavsFromFile(fileNameObj->value(),
        boatId).navs();
  }
  auto allNavs = concat(navs.get());
  std::sort(allNavs.begin(), allNavs.end());
  return NavCollection::fromNavs(allNavs);
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
  auto navs = loadNavs(amap, id);
  auto dstPath = getDstPath(amap);
  std::stringstream msg;
  msg << "Process " << navs.size() << " navs with boatid=" <<
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
  return processBoatData(debug, boatId, scanNmeaFolderWithSimulator(dataPath, boatId),
      PathBuilder(dataPath).pushDirectory("processed").get(), "all");
}











} /* namespace sail */
