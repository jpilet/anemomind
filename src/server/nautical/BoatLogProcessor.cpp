/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BoatLogProcessor.h"

#include <Poco/File.h>
#include <Poco/JSON/Stringifier.h>
#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>
#include <fstream>
#include <iostream>
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
#include <server/common/ArgMap.h>

#include <server/common/Json.impl.h>

namespace sail {


namespace {

using namespace sail;
using namespace std;

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
void visualizeBoatDat(Poco::Path dataPath) {
  auto boatDatPath = PathBuilder::makeDirectory(dataPath)
    .pushDirectory("processed").makeFile("boat.dat").get().toString();
  LOG(INFO) << "Load from " << boatDatPath;
  TargetSpeedTable table;
  CHECK(loadTargetSpeedTable(boatDatPath.c_str(), &table));
  dispTargetSpeedTable("Downwind", table, table._downwind);
  dispTargetSpeedTable("Upwind", table, table._upwind);
}

Nav::Id extractBoatId(Poco::Path path) {
  return path.directory(path.depth()-1);
}






void exampleUsage1Arg(std::stringstream *dst) {
  *dst << "     == Example usage one argument ==\n"
               "     Arg 1: testlogs/\n"
               "     \n"
      "     The program will recursively scan for all files in\n"
      "     the directory testlogs/ ending with .txt and try to\n"
      "     load NMEA data. It will then output the processed \n"
      "     data to files in the directory testlogs/processed/\n"
      "     on the format all_*.js\n\n";
}

void exampleUsage2Args(std::stringstream *dst) {
  *dst << "     == Example usage two arguments ==\n"
      "     Arg 1: testlogs/\n"
      "     Arg 2: IreneLog.txt\n"
               "\n"
      "     The program will load NMEA data from the file IreneLog.txt\n"
      "     the directory testlogs/. It will then output the processed \n"
      "     data to files in the directory testlogs/processed/\n"
      "     on the format IreneLog_*.js\n\n";
}

std::string makeHelpMessage() {
  std::stringstream ss;
  ss << "=== Anemomind Backend ===\n"
               "\n"
               "  This is a command line application for\n"
               "  the Anemomind backend.:\n"
               "  \n"
               "First argument:\n"
               "  A path where the last directory should\n"
               "  have the same name as the ID of the boat\n"
               "  whose data we want to optimize.\n"
               "\n"
               "Second argument (optional):\n"
               "  Name of log file to process. If omitted, all log files in the directory will be processed.\n";
               std::cout << std::endl;
   exampleUsage1Arg(&ss);
   exampleUsage2Args(&ss);
   return ss.str();
}

}

int continueProcessBoatLogs(ArgMap &amap) {
  auto args = amap.freeArgs();
  bool debug = amap.optionProvided("--debug");
  ENTERSCOPE("Anemomind boat log processor");
  if (args.empty()) {
    makeHelpMessage();
    return 0; //
  } else {
    std::string pathstr = args[0]->value();
    ENTERSCOPE("Process boat logs in directory " + pathstr);
    Poco::Path path = PathBuilder::makeDirectory(pathstr).get();
    if (args.size() == 1) {
      processBoatDataFullFolder(debug, path);
      if (debug) {
        visualizeBoatDat(path);
      }
      return 0;
    } else if (args.size() == 2) {
      std::string logFilename = args[1]->value();
      processBoatDataSingleLogFile(debug, path, logFilename);
      if (debug) {
        visualizeBoatDat(path);
      }
      return 0;
    } else {
      LOG(FATAL) << "Too many arguments";
      return -1;
    }
  }

}

int mainProcessBoatLogs(int argc, const char **argv) {
  ArgMap amap;
  amap.registerOption("--debug", "Display debug information and visualization");
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

namespace {
  TargetSpeed makeTargetSpeedTable(bool isUpwind,
      std::shared_ptr<HTree> tree, Array<HNode> nodeinfo,
      Array<Nav> allnavs,
      std::string description) {

    // TODO: How to best select upwind/downwind navs? Is the grammar reliable for this?
    //   Maybe replace AWA by TWA in order to label states in Grammar001.
    Arrayb sel = markNavsByDesc(tree, nodeinfo, allnavs, description);

    Array<Nav> navs = allnavs.slice(sel);

    Array<Velocity<double> > tws = estimateExternalTws(navs);
    Array<Velocity<double> > vmg = calcExternalVmg(navs, isUpwind);
    Array<Velocity<double> > gss = getGpsSpeed(navs);

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
}


void makeBoatDatFile(
    bool debug,
    PathBuilder outdir, Array<Nav> navs,
    std::shared_ptr<HTree> fulltree, Nav::Id boatId, WindOrientedGrammar g) {
  ENTERSCOPE("Output boat.dat with target speed data.");
  std::ofstream boatDatFile(outdir.makeFile("boat.dat").get().toString());

  Calibrator calibrator(g);
  if (calibrator.calibrate(navs, fulltree, boatId)) {
    calibrator.saveCalibration(&boatDatFile);
    calibrator.simulate(&navs);
  }

  outputTargetSpeedTable(debug, fulltree, g.nodeInfo(), navs, &boatDatFile);
}


void processBoatData(bool debug, Nav::Id boatId, Array<Nav> navs, Poco::Path dstPath, std::string filenamePrefix) {
  ENTERSCOPE("processBoatData");
  if (navs.size() == 0) {
    LOG(FATAL) << "No data to process.";
    return;
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

  makeBoatDatFile(debug, outdir, navs, fulltree, boatId, g);

  {
    std::string path = outdir.makeFile(prefix + "_tree.js").get().toString();
    ENTERSCOPE("Output tree (" + path + ")");
    ofstream file(path);
    Poco::JSON::Stringifier::stringify(json::serializeMapped(fulltree, navs, g.nodeInfo()), file, 0, 0);
  }{
    std::string path = outdir.makeFile(prefix + "_navs.js").get().toString();
    ENTERSCOPE("Output navs");
    ofstream file(path);
    Poco::JSON::Stringifier::stringify(json::serialize(navs), file, 0, 0);
  }{
    std::string path = outdir.makeFile(prefix + "_tree_node_info.js").get().toString();
    ENTERSCOPE("Output tree node info");
    ofstream file(path);
    Poco::JSON::Stringifier::stringify(json::serialize(g.nodeInfo()), file, 0, 0);
  }

}

void processBoatDataFullFolder(bool debug, Nav::Id boatId, Poco::Path srcPath, Poco::Path dstPath) {
  ENTERSCOPE("processBoatData complete folder");
  SCOPEDMESSAGE(INFO, std::string("Loading data from boat with id " + boatId));
  SCOPEDMESSAGE(INFO, "Scan folder for NMEA data...");
  Array<Nav> allnavs = scanNmeaFolderWithSimulator(srcPath, boatId);
  CHECK_LT(0, allnavs.size());
  SCOPEDMESSAGE(INFO, "done.")
  processBoatData(debug, boatId, allnavs, dstPath, "all");
}


/*
 * Processes the data related to a single boat.
 */
void processBoatDataFullFolder(bool debug, Poco::Path dataPath) {
  Nav::Id boatId = extractBoatId(dataPath);
  Poco::Path dataBuildDir = PathBuilder::makeDirectory(dataPath).pushDirectory("processed").get();
  processBoatDataFullFolder(debug, boatId, dataPath, dataBuildDir);
}

void processBoatDataSingleLogFile(bool debug, Nav::Id boatId, Poco::Path srcPath, std::string logFilename, Poco::Path dstPath) {
  ENTERSCOPE("processBoatData single log file");
  SCOPEDMESSAGE(INFO, std::string("Loading data from boat with id " + boatId));
  Poco::Path srcfile = PathBuilder::makeDirectory(srcPath).
        makeFile(logFilename).get();
  Array<Nav> navs = loadNavsFromFile(srcfile.toString(),
      boatId).navs();
  SCOPEDMESSAGE(INFO, "done.");
  std::sort(navs.begin(), navs.end());
  processBoatData(debug, boatId, navs, dstPath, srcfile.getBaseName());
}

// See discussion: https://github.com/jpilet/anemomind-web/pull/9#discussion_r12632698
void processBoatDataSingleLogFile(bool debug, Poco::Path dataPath, std::string logFilename) {
  Nav::Id boatId = extractBoatId(dataPath);
  Poco::Path dataBuildDir = PathBuilder::makeDirectory(dataPath).pushDirectory("processed").get();
  processBoatDataSingleLogFile(debug, boatId, dataPath, logFilename, dataBuildDir);
}



} /* namespace sail */
