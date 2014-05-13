/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

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


namespace sail {


namespace {

using namespace sail;
using namespace Poco::Util;
using namespace std;


Nav::Id extractBoatId(Poco::Path path) {
  return path.directory(path.depth()-1);
}







void dispHelp() {
  std::cout << "Anemomind Backend\n"
               "\n"
               "This is a command line application for\n"
               "the Anemomind backend. It is called with\n"
               "a single argument:\n"
               "\n"
               "A path where the last directory should\n"
               "have the same name as the ID of the boat\n"
               "whose data we want to optimize.";
               std::cout << std::endl;
}

}


int BoatLogProcessor::main(const std::vector<std::string>& args) {
  ENTERSCOPE("Anemomind boat log processor");
  if (args.empty()) {
    dispHelp();
    return Application::EXIT_NOINPUT;
  } else {
    if (args.size() == 1) {
      std::string pathstr = args[0];
      ENTERSCOPE("Process boat logs in directory " + pathstr);
      Poco::Path path = PathBuilder::makeDirectory(pathstr).get();
      processBoatData(path);
    } else {
      dispHelp();
      LOG(FATAL) << "Wrong number of arguments";
    }
    return Application::EXIT_IOERR;
  }
}

void processBoatData(Nav::Id boatId, Poco::Path srcPath, Poco::Path dstPath) {
  ENTERSCOPE(__FUNCTION__);
  SCOPEDMESSAGE(INFO, std::string("Loading data from boat with id " + boatId));

  SCOPEDMESSAGE(INFO, "Scan folder for NMEA data...");
  Array<Nav> allnavs = scanNmeaFolder(srcPath, boatId);
  CHECK_LT(0, allnavs.size());
  SCOPEDMESSAGE(INFO, "done.")

  Grammar001Settings settings;
  Grammar001 g(settings);

  SCOPEDMESSAGE(INFO, "Parse data...");
  std::shared_ptr<HTree> fulltree = g.parse(allnavs); //allnavs.sliceTo(2000));
  SCOPEDMESSAGE(INFO, "done.");

  Poco::File buildDir(dstPath);

  // It could be that the directory already exists, and in that case,
  // we are going to overwrite previous data.
  buildDir.createDirectory();

  std::string prefix = PathBuilder::makeDirectory(dstPath).makeFile("output").get().toString();


  {
    ENTERSCOPE("Output tree");
   ofstream file(prefix + "_tree.js");
   json::serializeMapped(fulltree, allnavs, g.nodeInfo())->stringify(file, 0, 0);
  }{
    ENTERSCOPE("Output navs");
   ofstream file(prefix + "_navs.js");
   json::serialize(allnavs).stringify(file, 0, 0);
  }{
    ENTERSCOPE("Output tree node info");
   ofstream file(prefix + "_tree_node_info.js");
   json::serialize(g.nodeInfo()).stringify(file, 0, 0);
  }
}



/*
 * Processes the data related to a single boat.
 */
void processBoatData(Poco::Path dataPath) {
  Nav::Id boatId = extractBoatId(dataPath);
  Poco::Path dataBuildDir = PathBuilder::makeDirectory(dataPath).pushDirectory("build").get();
  processBoatData(boatId, dataPath, dataBuildDir);
}


} /* namespace sail */
