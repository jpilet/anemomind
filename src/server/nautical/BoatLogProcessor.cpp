/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/CommonJson.h>
#include "BoatLogProcessor.h"
#include <iostream>
#include <Poco/Util/Application.h>
#include <Poco/File.h>
#include <server/common/PathBuilder.h>
#include <server/common/logging.h>
#include <server/common/ScopedLog.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/string.h>
#include <server/common/HierarchyJson.h>
#include <server/nautical/NavJson.h>
#include <fstream>
#include <server/nautical/grammars/Grammar001.h>
#include <server/nautical/HTreeJson.h>
#include <server/nautical/NavNmea.h>


namespace sail {


namespace {

using namespace sail;
using namespace Poco::Util;
using namespace std;


Nav::Id extractBoatId(Poco::Path path) {
  return path.directory(path.depth()-1);
}





void exampleUsage1Arg() {
  std::cout << "     == Example usage one argument ==\n"
               "     Arg 1: testlogs/\n"
               "     \n"
      "     The program will recursively scan for all files in\n"
      "     the directory testlogs/ ending with .txt and try to\n"
      "     load NMEA data. It will then output the processed \n"
      "     data to files in the directory testlogs/processed/\n"
      "     on the format all_*.js\n\n";
}

void exampleUsage2Args() {
  std::cout << "     == Example usage two arguments ==\n"
      "     Arg 1: testlogs/\n"
      "     Arg 2: IreneLog.txt\n"
               "\n"
      "     The program will load NMEA data from the file IreneLog.txt\n"
      "     the directory testlogs/. It will then output the processed \n"
      "     data to files in the directory testlogs/processed/\n"
      "     on the format IreneLog_*.js\n\n";
}

void dispHelp() {
  std::cout << "=== Anemomind Backend ===\n"
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
   exampleUsage1Arg();
   exampleUsage2Args();
}

}


int BoatLogProcessor::main(const std::vector<std::string>& args) {
  ENTERSCOPE("Anemomind boat log processor");
  if (args.empty()) {
    dispHelp();
    return Application::EXIT_NOINPUT;
  } else {

    if (args.size() == 0) {
      dispHelp();
      LOG(FATAL) << "Too few arguments";
    } else {
      std::string pathstr = args[0];
      ENTERSCOPE("Process boat logs in directory " + pathstr);
      Poco::Path path = PathBuilder::makeDirectory(pathstr).get();
      if (args.size() == 1) {
        processBoatDataFullFolder(path);
        return Application::EXIT_OK;
      } else {
        std::string logFilename = args[1];
        if (args.size() == 2) {
          processBoatDataSingleLogFile(path, logFilename);
          return Application::EXIT_OK;
        } else {
          LOG(FATAL) << "Too many arguments";
        }
      }
    }
    return Application::EXIT_IOERR;
  }
}



void processBoatData(Nav::Id boatId, Array<Nav> navs, Poco::Path dstPath, std::string filenamePrefix) {
  ENTERSCOPE("processBoatData");
  SCOPEDMESSAGE(INFO, stringFormat("Process %d navs ranging from %s to %s",
      navs.size(), navs.first().time().toString().c_str(),
      navs.last().time().toString().c_str()));
  Grammar001Settings settings;
  Grammar001 g(settings);

  SCOPEDMESSAGE(INFO, "Parse data...");
  std::shared_ptr<HTree> fulltree = g.parse(navs); //allnavs.sliceTo(2000));
  SCOPEDMESSAGE(INFO, "done.");

  Poco::File buildDir(dstPath);

  // It could be that the directory already exists, and in that case,
  // we are going to overwrite previous data.
  buildDir.createDirectory();

  std::string prefix = PathBuilder::makeDirectory(dstPath).makeFile(filenamePrefix).get().toString();


  {
    ENTERSCOPE("Output tree");
   ofstream file(prefix + "_tree.js");
   json::serializeMapped(fulltree, navs, g.nodeInfo())->stringify(file, 0, 0);
  }{
    ENTERSCOPE("Output navs");
   ofstream file(prefix + "_navs.js");
   json::serialize(navs)->stringify(file, 0, 0);
  }{
    ENTERSCOPE("Output tree node info");
   ofstream file(prefix + "_tree_node_info.js");
   json::serialize(g.nodeInfo())->stringify(file, 0, 0);
  }
}

void processBoatDataFullFolder(Nav::Id boatId, Poco::Path srcPath, Poco::Path dstPath) {
  ENTERSCOPE("processBoatData complete folder");
  SCOPEDMESSAGE(INFO, std::string("Loading data from boat with id " + boatId));
  SCOPEDMESSAGE(INFO, "Scan folder for NMEA data...");
  Array<Nav> allnavs = scanNmeaFolder(srcPath, boatId);
  CHECK_LT(0, allnavs.size());
  SCOPEDMESSAGE(INFO, "done.")
  processBoatData(boatId, allnavs, dstPath, "all");
}


/*
 * Processes the data related to a single boat.
 */
void processBoatDataFullFolder(Poco::Path dataPath) {
  Nav::Id boatId = extractBoatId(dataPath);
  Poco::Path dataBuildDir = PathBuilder::makeDirectory(dataPath).pushDirectory("processed").get();
  processBoatDataFullFolder(boatId, dataPath, dataBuildDir);
}

void processBoatDataSingleLogFile(Nav::Id boatId, Poco::Path srcPath, std::string logFilename, Poco::Path dstPath) {
  ENTERSCOPE("processBoatData single log file");
  SCOPEDMESSAGE(INFO, std::string("Loading data from boat with id " + boatId));
  //loadNavsFromNmea(files[i].toString(), boatId);
  Poco::Path srcfile = PathBuilder::makeDirectory(srcPath).
        makeFile(logFilename).get();
  Array<Nav> navs = loadNavsFromNmea(srcfile.toString(),
      boatId).navs();
  SCOPEDMESSAGE(INFO, "done.");
  std::sort(navs.begin(), navs.end());
  processBoatData(boatId, navs, dstPath, srcfile.getBaseName());
}


// See discussion: https://github.com/jpilet/anemomind-web/pull/9#discussion_r12632698
void processBoatDataSingleLogFile(Poco::Path dataPath, std::string logFilename) {
  Nav::Id boatId = extractBoatId(dataPath);
  Poco::Path dataBuildDir = PathBuilder::makeDirectory(dataPath).pushDirectory("processed").get();
  processBoatDataSingleLogFile(boatId, dataPath, logFilename, dataBuildDir);
}



} /* namespace sail */
