/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BoatLogProcessor.h"


#include <Poco/File.h>
#include <Poco/JSON/Stringifier.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>
#include <device/anemobox/simulator/SimulateBox.h>
#include <fstream>
#include <iostream>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/ScopedLog.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/nautical/DownsampleGps.h>
#include <server/nautical/MaxSpeed.h>
#include <server/nautical/TargetSpeed.h>
#include <server/nautical/calib/Calibrator.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/grammars/TreeExplorer.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/tiles/ChartTiles.h>
#include <server/nautical/tiles/TileUtils.h>
#include <server/plot/extra.h>
#include <server/nautical/BoatSpecificHacks.h>

namespace sail {

using namespace NavCompat;

namespace {

using namespace sail;
using namespace std;

namespace {

bool debugVmgSamples = false;

void collectSpeedSamplesGrammar(
      std::shared_ptr<HTree> tree, Array<HNode> nodeinfo,
      const NavDataset& allnavs,
      std::string description,
      Array<Velocity<>> *twsResult,
      Array<Velocity<>> *vmgResult) {
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
      TimedSampleRange<Angle<double>> twaLeg = leg.samples<TWA>();
      TimedSampleRange<Angle<double>> awaLeg = leg.samples<AWA>();

      for (TimeStamp time(it.first); time < it.second; time += Duration<>::seconds(1)) {
        Optional<TimedValue<Velocity<>>> tws = twsLeg.evaluate(time);
        Optional<TimedValue<Velocity<>>> vmg = vmgLeg.evaluate(time);

        if (tws.defined() && vmg.defined()) {
          // We ignore samples with low VMG.
          // This is because the grammar labels as "upwind-leg" startionary
          // episodes.
          if (vmg.get().value.fabs() > .5_kn) {
            twsArray.add(tws.get().value);
            // vmg is negative for downwind sailing, but the TargetSpeed logic
            // only handles positive values. Therefore, take the abs value.
            vmgArray.add(fabs(vmg.get().value));
            if (debugVmgSamples) {
            LOG(INFO) << "At " << time.fullPrecisionString() << ": "
              << "vmg: " << vmg.get().value.knots()
              << " gpsSpeed: " << leg.samples<GPS_SPEED>().evaluate(time).get().value.knots()
              << " tws: " << tws.get().value.knots()
              << " twa: " << twaLeg.evaluate(time).get().value.degrees()
              << " awa: " << awaLeg.evaluate(time).get().value.degrees()
              << " aws: " << leg.samples<AWS>().evaluate(time).get().value.knots();
            }
          } else {
            if (debugVmgSamples) {
              LOG(INFO) << "At " << time.fullPrecisionString()
                << ": ignoring sample with VMG " << vmg.get().value.knots();
            }
          }
        }
      }
    }

    *twsResult = twsArray.get();
    *vmgResult = vmgArray.get();

    LOG(INFO) << __FUNCTION__ << ": "
      << " " << description << ": " << sel.size() << " legs "
      << twsResult->size() << " measures";
}


void collectSpeedSamplesBlind(const NavDataset& navs,
                              bool isUpwind,
                              Array<Velocity<>> *twsResult,
                              Array<Velocity<>> *vmgResult) {
  TimedSampleRange<Angle<double>> allTwa = navs.samples<TWA>();
  TimedSampleRange<Velocity<double>> allGpsSpeed = navs.samples<GPS_SPEED>();
  TimedSampleRange<Velocity<double>> allVmg = navs.samples<VMG>();
  TimedSampleRange<Velocity<double>> allTws = navs.samples<TWS>();

  ArrayBuilder<Velocity<double>> twsArray;
  ArrayBuilder<Velocity<double>> vmgArray;

  if (allGpsSpeed.size() == 0) {
    return;
  }

  const Duration<> maxDelta = Duration<>::seconds(3);

  for (int i = 0; i < allGpsSpeed.size(); ++i) {
    TimeStamp time = allGpsSpeed[i].time;
    Velocity<double> speed = allGpsSpeed[i].value;
    if (speed < Velocity<>::knots(.7)) {
      // the boat is almost static... let's ignore this measure.
      continue;
    }
    Optional<TimedValue<Velocity<>>> tws = allTws.evaluate(time);
    Optional<TimedValue<Velocity<>>> vmg = allVmg.evaluate(time);
    Optional<TimedValue<Angle<>>> twa = allTwa.evaluate(time);

    if (tws.undefined() || vmg.undefined() || twa.undefined()
        || fabs(tws.get().time - time) > maxDelta
        || fabs(vmg.get().time - time) > maxDelta
        || fabs(twa.get().time - time) > maxDelta) {
      // measures do not align... ignore.
      continue;
    }
    bool upwind = cos(twa.get().value) > 0;

    if (isUpwind == upwind) {
      twsArray.add(tws.get().value);
      // vmg is negative for downwind sailing, but the TargetSpeed logic
      // only handles positive values. Therefore, take the abs value.
      vmgArray.add(fabs(vmg.get().value));
    }
  }
  *twsResult = twsArray.get();
  *vmgResult = vmgArray.get();

  LOG(INFO) << __FUNCTION__ << ": " << (isUpwind ? "upwind" : "downwind")
    << twsResult->size() << " measures";
}

  TargetSpeed makeTargetSpeedTable(
      bool isUpwind,
      VmgSampleSelection vmgSampleSelection,
      std::shared_ptr<HTree> tree, Array<HNode> nodeinfo,
      const NavDataset& allnavs,
      std::string description) {
    Array<Velocity<>> twsArr;
    Array<Velocity<>> vmgArr;

    switch (vmgSampleSelection) {
      case VMG_SAMPLES_FROM_GRAMMAR:
        collectSpeedSamplesGrammar(tree, nodeinfo, allnavs, description,
                                   &twsArr, &vmgArr);
        break;
      case VMG_SAMPLES_BLIND:
        collectSpeedSamplesBlind(allnavs, isUpwind, &twsArr, &vmgArr);
        break;
    }

    // TODO: Adapt these values to the amount of recorded data.
    Velocity<double> minvel = Velocity<double>::knots(0);
    Velocity<double> maxvel = Velocity<double>::knots(TargetSpeedTable::NUM_ENTRIES-1);
    Array<Velocity<double> > bounds = makeBoundsFromBinCenters(TargetSpeedTable::NUM_ENTRIES, minvel, maxvel);
    return TargetSpeed(isUpwind, twsArr, vmgArr, bounds);
  }

  void outputTargetSpeedTable(
      bool debug,
      std::shared_ptr<HTree> tree,
      Array<HNode> nodeinfo,
      NavDataset navs,
      VmgSampleSelection vmgSampleSelection,
      std::ofstream *file) {
    TargetSpeed uw = makeTargetSpeedTable(true, vmgSampleSelection,
                                          tree, nodeinfo, navs, "upwind-leg");
    TargetSpeed dw = makeTargetSpeedTable(false, vmgSampleSelection,
                                          tree, nodeinfo, navs, "downwind-leg");

    if (debug) {
      LOG(INFO) << "Upwind";
      uw.plot();
      LOG(INFO) << "Downwind";
      dw.plot();
    }

    saveTargetSpeedTableChunk(file, uw, dw);
  }

}  // namespace

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

std::string grammarNodeInfo(const NavDataset& navs, std::shared_ptr<HTree> tree) {
  CHECK(tree->left() < tree->right());
  Nav right = getNav(navs, tree->right()-1);
  Nav left = getNav(navs, tree->left());
  return left.time().toString() + " to "
      + right.time().toString()
      + " with duration of " + (right.time() - left.time()).str();
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

void BoatLogProcessor::grammarDebug(
    const std::shared_ptr<HTree> &fulltree,
    const NavDataset &resampled) const {
  auto grammarNodeInfoResampled =
      [&](std::shared_ptr<HTree> t) {return grammarNodeInfo(resampled, t);};
  if (_exploreGrammar) {
    exploreTree(
        _grammar.grammar.nodeInfo(), fulltree, &std::cout,
        grammarNodeInfoResampled);
  }
  if (_logGrammar) {
    std::ofstream file(_dstPath.toString() + "/loggrammar.txt");
    outputLogGrammar(&file, _grammar.grammar.nodeInfo(),
        fulltree, grammarNodeInfoResampled);
  }
}

void outputSessionSummary(const NavDataset &ds, DOM::Node *dst) {
  Optional<TimedValue<Velocity<>>> instant = computeInstantMaxSpeed(ds);
  Optional<TimedValue<Velocity<>>> period = computeMaxSpeedOverPeriod(ds);
  DOM::addSubTextNode(dst, "li",
      stringFormat("Max speed instant: %.3g knots, over period: %.3g",
          instant.defined()? instant.get().value.knots() : 0.0,
          period.defined()? period.get().value.knots() : 0.0));
}

void outputInfoPerSession(
    const Array<NavDataset> &sessions,
    DOM::Node *log) {
  DOM::addSubTextNode(log, "h2", "Sessions");
  auto ul = DOM::makeSubNode(log, "ul");
  for (auto s: sessions) {
    outputSessionSummary(s, &ul);
  }
}


//
// high-level processing logic
//
// The goal of this function is to stay small and easy to read,
// while explicitly showing what is going on.
// Parameters such as path to files are passed implicitly (as struct
// members), while raw and derived data are kept in local variables,
// to improve data flow readability.

bool BoatLogProcessor::process(ArgMap* amap) {
  TimeStamp start = TimeStamp::now();

  if (!prepare(amap)) {
    return false;
  }

  if (_boatid == "59b1343a0411db0c8d8fbf7c") {
    hack::forceDateForGLL = true;
  }

  NavDataset current;

  if (_resumeAfterPrepare.size() > 0) {
    current = LogLoader::loadNavDataset(_resumeAfterPrepare);
  } else {
    current = removeStrangeGpsPositions(
        loadNavs(*amap, _boatid));
    infoNavDataset("After loading", current);

    auto minGpsSamplingPeriod = 0.01_s; // Should be enough, right?
    current = current.createMergedChannels(
        std::set<DataCode>{GPS_POS, GPS_SPEED, GPS_BEARING},
        minGpsSamplingPeriod);
    infoNavDataset("After resampling GPS", current);

    if (_gpsFilter) {
      current = filterNavs(current, &_htmlReport, _gpsFilterSettings);
      infoNavDataset("After filtering", current);
    }
  }

  if (_savePreparedData.size() != 0) {
    saveDispatcher(_savePreparedData.c_str(), *(current.dispatcher()));
  }

  // Note: the grammar does not have access to proper true wind.
  // It has to do its own estimate.
  current = current.createMergedChannels(
      std::set<DataCode>{AWA, AWS}, Duration<>::seconds(.3));
  std::shared_ptr<HTree> fulltree = _grammar.parse(current);

  if (!fulltree) {
    LOG(WARNING) << "grammar parsing failed. No data? boat: " << _boatid;
    return false;
  }

  grammarDebug(fulltree, current);

  Calibrator calibrator(_grammar.grammar);
  if (_verboseCalibrator) { calibrator.setVerbose(); }
  std::string boatDatPath = _dstPath.toString() + "/boat.dat";
  std::ofstream boatDatFile(boatDatPath);
  CHECK(boatDatFile.is_open()) << "Error opening " << boatDatPath;

  // Calibrate. TODO: use filtered data instead of resampled.
  if (calibrator.calibrate(current, fulltree, _boatid)) {
      calibrator.saveCalibration(&boatDatFile);
  } else {
    LOG(WARNING) << "Calibration failed. Using default calib values.";
    calibrator.clear();
    if (_saveDefaultCalib) {
      calibrator.saveCalibration(&boatDatFile);
    }
  }

  // First simulation pass: adds true wind
  current = calibrator.simulate(current);

  // This choice should be left to the user.
  // TODO: add a per-boat configuration system
  current = current.preferSourceOrCreateMergedChannels(
      std::set<DataCode>{TWS, TWDIR, TWA, VMG},
      "Simulated Anemomind estimator");

  if (_saveSimulated.size() > 0) {
    saveDispatcher(_saveSimulated.c_str(), *(current.dispatcher()));
  }

  outputTargetSpeedTable(_debug, 
                         fulltree,
                         _grammar.grammar.nodeInfo(),
                         current,
                         _vmgSampleSelection,
                         &boatDatFile);

  // write calibration and target speed to disk
  boatDatFile.close();

  // Second simulation path to apply target speed.
  // Todo: simply lookup the target speed instead of recomputing true wind.
  current = SimulateBox(boatDatPath, current);

  if (_debug) {
    visualizeBoatDat(_dstPath);
  }

  HTML_DISPLAY(_generateTiles, &_htmlReport);
  if (_generateTiles) {
    Array<NavDataset> sessions =
      extractAll("Sailing", current, _grammar.grammar, fulltree);
    outputInfoPerSession(sessions, &_htmlReport);
    if (!generateAndUploadTiles(_boatid, sessions, db.db, _tileParams)) {
      LOG(ERROR) << "generateAndUpload: tile generation failed";
      return false;
    }
  }

  HTML_DISPLAY(_generateChartTiles, &_htmlReport);
  if (_generateChartTiles) {
    if (!uploadChartTiles(
        current, _boatid, _chartTileSettings, db.db)
        || !uploadChartSourceIndex(
            current, _boatid, _chartTileSettings, db.db)) {
      LOG(ERROR) << "Failed to upload chart tiles!";
      return false;
    }
  }
  // Logging to cout and not LOG(INFO) because LOG(INFO) is disabled in
  // production and we want to keep track of processing time.
  std::cout << "Processing time for " << _boatid << ": "
    << (TimeStamp::now() - start).seconds() << " seconds." << std::endl;
  return true;
}

void BoatLogProcessor::infoNavDataset(const std::string& info,
                                      const NavDataset& ds) {
  if (_debug) {
    std::cout << info << ": ";
    ds.outputSummary(&std::cout);
  }
  DOM::addSubTextNode(&_htmlReport, "h2", info);
  std::stringstream ss;
  ds.outputSummary(&ss);

  auto s = summarizeDispatcherOverTime(
      ds.dispatcher().get(),
      roundOffToBin(15.0_minutes));
  std::set<DataCode> ofInterest{AWA, TWA, AWS, TWS};
  for (const auto& kv: s) {
    ss << "At "<< kv.first.toString() << std::endl;
    for (const auto& codeSourceAndCount: kv.second) {
      auto codeSource = codeSourceAndCount.first;
      int n = codeSourceAndCount.second;
      if (true || 1 <= ofInterest.count(codeSource.first)) {
        ss << "   " << wordIdentifierForCode(codeSource.first)
            << ", " << codeSource.second << ": " << n << std::endl;
      }
    }
  }


  DOM::addSubTextNode(&_htmlReport, "pre", ss.str());
}

void BoatLogProcessor::readArgs(ArgMap* amap) {
  _debug = amap->optionProvided("--debug");
  _boatid = getBoatId(*amap);
  _dstPath = getDstPath(*amap);
  _generateTiles = amap->optionProvided("-t");
  _generateChartTiles = amap->optionProvided("-c");
  _vmgSampleSelection = (amap->optionProvided("--vmg:blind") ?
    VMG_SAMPLES_BLIND : VMG_SAMPLES_FROM_GRAMMAR);

  _gpsFilter = !amap->optionProvided("--no-gps-filter");

  _tileParams.fullClean = amap->optionProvided("--clean");

  _exploreGrammar = amap->optionProvided("--explore");
  _logGrammar = amap->optionProvided("--log-grammar");

  _chartTileSettings.dbName = _tileParams.dbName();
  if (_debug) {
    LOG(INFO) << "BoatLogProcessor:\n"
      << "boat: " << _boatid << "\n"
      << "dst: " << _dstPath.toString() << "\n"
      << (_generateTiles ? "gen tiles" : " no tile gen") << "\n"
      << (_vmgSampleSelection == VMG_SAMPLES_FROM_GRAMMAR ?
          "grammar vmg samples" : "blind vmg samples");
  }

  _tileParams.curveCutThreshold = _gpsFilterSettings.subProblemThreshold;
}

bool BoatLogProcessor::prepare(ArgMap* amap) {
  readArgs(amap);

  if (!_htmlReportName.empty()) {
    _htmlReport = DOM::makeBasicHtmlPage("Boat log processor",
          _dstPath.toString(), _htmlReportName);
    _tileParams.log = _htmlReport;
  }

  if (_generateTiles || _generateChartTiles) {
    db = MongoDBConnection(_tileParams.uri());
    if (!db.defined()) {
      return false;
    }
  }

  return true;
}

int mainProcessBoatLogs(int argc, const char **argv) {
  BoatLogProcessor processor;
  ArgMap amap;
  amap.setHelpInfo("Help for boat log processor");

  amap.registerOption("--debug", "Display debug information and visualization")
    .setArgCount(0);

  amap.registerOption("--output-html",
      "Produce a HTML report with the specified name in the output directory")
    .setArgCount(1).store(&processor._htmlReportName);

  amap.registerOption("--saveSimulated <file.log>",
                      "Save dispatcher in the given file after simulation")
    .store(&processor._saveSimulated);

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

  amap.registerOption(
      "--vmg:blind", "Instead of using grammar to find legs for vmg samples, use everything.")
    .setArgCount(0);

  amap.registerOption("--no-gps-filter", "skip gps filtering").setArgCount(0);

  amap.disableFreeArgs();

  TileGeneratorParameters* params = &processor._tileParams;

  amap.registerOption("-t", "Generate vector tiles and upload to mongodb")
    .setArgCount(0);

  amap.registerOption("-c", "Generate chart tiles and upload to mongodb")
    .setArgCount(0);

  amap.registerOption("--mongo-uri", "Full URI to Mongo DB")
      .store(&params->mongoUri);
  amap.registerOption("--scale", "max scale level").store(&params->maxScale);
  amap.registerOption("--maxpoints",
                      "downsample curves if they have more than <maxpoints> points")
    .store(&params->maxNumNavsPerSubCurve);


  amap.registerOption("--clean", "Clean all tiles for this boat before starting");

  amap.registerOption(
      "--debug-vmg",
      "Print detailed information about samples used for VMG target speed tables")
    .store(&debugVmgSamples);

  amap.registerOption(
      "--save-prepared",
      "Save after downsampling and early filtering, see --continue-prepared")
    .store(&processor._savePreparedData);

  amap.registerOption("--continue-prepared",
                      "continue processing on a file saved with --save-prepared")
    .store(&processor._resumeAfterPrepare);

  amap.registerOption("--verbose-calib", "Enable debug output for calibration")
    .store(&processor._verboseCalibrator);

  amap.registerOption("--save-default-calib", "Save default calibration values even if calibration failed")
    .store(&processor._saveDefaultCalib);

  amap.registerOption("--explore", "Explore grammar tree")
    .store(&processor._exploreGrammar);

  amap.registerOption("--log-grammar",
      "Produce a log file with the parsed result");

  auto status = amap.parse(argc, argv);
  switch (status) {
   case ArgMap::Error:
     return -1;
   case ArgMap::Done:
     return 0;
   case ArgMap::Continue:
     return processor.process(&amap) ? 0 : 1;
   default:
     LOG(FATAL) << "Unhandled ParseStatus code";
     return -1;
  };
}

} /* namespace sail */
