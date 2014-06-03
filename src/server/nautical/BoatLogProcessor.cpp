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
#include <server/nautical/NavNmea.h>
#include <Poco/JSON/Stringifier.h>
#include <server/nautical/Calibrator.h>
#include <server/nautical/TargetSpeed.h>
#include <server/common/Span.h>
#include <server/plot/extra.h>
#include <server/common/ArrayIO.h>


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
               "  Name of log file to process. If omitted, \n"
               "  all log files in the directory will be processed.\n";
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

namespace {

  Array<Velocity<double> > estimateTws(Calibrator &c, Array<Nav> navs) {
    return navs.map<Velocity<double> >([&] (const Nav &n) {
      return Calibrator::WindEstimator::computeTrueWind(c.calibrationValues(), n).norm();
    });
  }

  Angle<double> estimateRawTwa(Calibrator &c, const Nav &n) {
    return Calibrator::WindEstimator::computeTrueWind(c.calibrationValues(), n).angle() -
        n.gpsBearing(); // <-- Should it be magHdg or gpsBearing here?
  }


  Array<Velocity<double> > calcVmg(Calibrator &c, Array<Nav> navs, bool isUpwind) {
    int sign = (isUpwind? 1 : -1);
    return navs.map<Velocity<double> >([&](const Nav &n) {
      Angle<double> twa = estimateRawTwa(c, n);
      double factor = sign*cos(twa);
      return n.gpsSpeed().scaled(factor);
    });
  }

  TargetSpeedData makeTargetSpeedData(bool isUpwind, Calibrator &c, Array<Nav> navs) {
//    Array<Velocity<double> > vmg = calcVmg(navs, isUpwind);
//    Array<Velocity<double> > tws = estimateTws(navs);

    Array<Velocity<double> > tws = estimateTws(c, navs);
    Array<Velocity<double> > vmg = calcVmg(c, navs, isUpwind);
    Array<Velocity<double> > gss = getGpsSpeed(navs);

    { // For debugging, if needed.
      Arrayd gss_mps = gss.map<double>([&](Velocity<double> x) {return x.metersPerSecond();});
      Arrayd tws_mps = tws.map<double>([&](Velocity<double> x) {return x.metersPerSecond();});
      Arrayd vmg_mps = vmg.map<double>([&](Velocity<double> x) {return x.metersPerSecond();});
      LOG(INFO) << EXPR_AND_VAL_AS_STRING(navs.size());
      LOG(INFO) << EXPR_AND_VAL_AS_STRING(isUpwind);
      LOG(INFO) << EXPR_AND_VAL_AS_STRING(Spand(gss_mps));
      LOG(INFO) << EXPR_AND_VAL_AS_STRING(Spand(tws_mps));
      LOG(INFO) << EXPR_AND_VAL_AS_STRING(Spand(vmg_mps));
      //GnuplotExtra plot;
      //plot.plot_xy(tws_mps, vmg_mps);
      //plot.show();
      int count = tws.size();
      MDArray2d data(count, 2);
      for (int i = 0; i < count; i++) {
        data(i, 0) = tws_mps[i];
        data(i, 1) = gss_mps[i];
      }
      saveMatrix(stringFormat("/home/jonas/temp%d.txt", isUpwind), data);
    }

    // TODO: Choose these parameters carefully
    const int binCount = 25;
    Velocity<double> minvel = Velocity<double>::metersPerSecond(4.0);
    Velocity<double> maxvel = Velocity<double>::metersPerSecond(17.0);

    return TargetSpeedData(tws, vmg, binCount,
        minvel, maxvel);
  }
}

namespace {
  bool majorityIsUpwind(Calibrator &c, Array<Nav> navs) {
    int count = 0;
    for (auto n : navs) {
      if (cos(estimateRawTwa(c, n)) > 0) {
        count++;
      }
    }
    std::cout << "Upwind fraction: " << double(count)/navs.size() << std::endl;
    return 2*count > navs.size();
  }
}

Arrayb selectUpwind(Calibrator &c, Array<Nav> navs) {
  return navs.map<bool>([&](const Nav &n) {
    return cos(estimateRawTwa(c, n)) > 0;
  });
}

void computeTargetSpeedData(Calibrator &c, Array<Nav> allnavs, TargetSpeedData *uw, TargetSpeedData *dw) {
  CHECK(c.empty() || c.allnavs().size() == allnavs.size());
  auto v = [&](const Nav &n) {
      return Calibrator::WindEstimator::valid(n);
    };
  Array<Nav> navs = allnavs.slice(v);
  Arrayb upwind = selectUpwind(c, navs);
  Arrayb downwind = neg(upwind);
//  Arrayb upwind = markNavsByDesc(c.tree(), c.grammar().nodeInfo(),
//      navs, "upwind-leg");
//  Arrayb downwind = markNavsByDesc(c.tree(), c.grammar().nodeInfo(),
//          navs, "downwind-leg");
  Array<Nav> uwNavs = navs.slice(upwind).slice(v);
  Array<Nav> dwNavs = navs.slice(downwind).slice(v);
  assert(majorityIsUpwind(c, uwNavs));
  assert(!majorityIsUpwind(c, dwNavs));
  *uw = makeTargetSpeedData(true,  c, uwNavs);
  *dw = makeTargetSpeedData(false, c, dwNavs);
}

namespace {
  void outputTargetSpeedData(Calibrator &c, std::string prefix) {
    TargetSpeedData uw, dw;
    computeTargetSpeedData(c, c.allnavs(), &uw, &dw);
    std::ofstream file(prefix + "_target_speed.dat");
    saveTargetSpeedTableChunk(&file, uw, dw);
  }
}

void processBoatData(Nav::Id boatId, Array<Nav> navs,
    Poco::Path dstPath, std::string filenamePrefix) {
  ENTERSCOPE("processBoatData");
  SCOPEDMESSAGE(INFO, stringFormat("Process %d navs ranging from %s to %s",
      navs.size(), navs.first().time().toString().c_str(),
      navs.last().time().toString().c_str()));

  Calibrator calibrator;
  calibrator.calibrate(navs);

  SCOPEDMESSAGE(INFO, "Parse data...");
  std::shared_ptr<HTree> fulltree = calibrator.tree(); //allnavs.sliceTo(2000));
  SCOPEDMESSAGE(INFO, "done.");

  Poco::File buildDir(dstPath);

  // It could be that the directory already exists, and in that case,
  // we are going to overwrite previous data.
  buildDir.createDirectory();

  std::string prefix = PathBuilder::makeDirectory(dstPath).
      makeFile(filenamePrefix).get().toString();


  {
    ENTERSCOPE("Output tree");
   ofstream file(prefix + "_tree.js");
   Poco::JSON::Stringifier::stringify(json::serializeMapped(fulltree,
       navs, calibrator.grammar().nodeInfo()), file, 0, 0);
  }{
    ENTERSCOPE("Output navs");
   ofstream file(prefix + "_navs.js");
   Poco::JSON::Stringifier::stringify(json::serialize(navs), file, 0, 0);
  }{
    ENTERSCOPE("Output tree node info");
   ofstream file(prefix + "_tree_node_info.js");
   Poco::JSON::Stringifier::stringify(json::serialize(
       calibrator.grammar().nodeInfo()), file, 0, 0);
  }
  outputTargetSpeedData(calibrator, prefix);
}

void processBoatDataFullFolder(Nav::Id boatId,
    Poco::Path srcPath, Poco::Path dstPath) {
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

void processBoatDataSingleLogFile(Nav::Id boatId,
    Poco::Path srcPath, std::string logFilename, Poco::Path dstPath) {
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
