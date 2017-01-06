/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BoatLogProcessor.h"


#include <Poco/File.h>
#include <Poco/JSON/Stringifier.h>
#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>
#include <device/anemobox/simulator/SimulateBox.h>
#include <fstream>
#include <iostream>
#include <server/common/Env.h>
#include <server/common/HierarchyJson.h>
#include <server/common/Json.h>
#include <server/common/PathBuilder.h>
#include <server/common/ScopedLog.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/nautical/DownsampleGps.h>
#include <server/nautical/HTreeJson.h>
#include <server/nautical/NavJson.h>
#include <server/nautical/TargetSpeed.h>
#include <server/nautical/calib/Calibrator.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/tiles/ChartTiles.h>
#include <server/nautical/tiles/TileUtils.h>
#include <server/plot/extra.h>

#include <server/common/Json.impl.h> // This one should probably be the last one.

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

//
// high-level processing logic
//
// The goal of this function is to stay small and easy to read,
// while explicitely showing what is going on.
// Parameters such as path to files are passed implicitely (as struct
// members), while raw and derived data are kept in local variables,
// to improve data flow readability.
bool BoatLogProcessor::process(ArgMap* amap) {
  TimeStamp start = TimeStamp::now();

  if (!prepare(amap)) {
    return false;
  }

  NavDataset resampled;

  if (_resumeAfterPrepare.size() > 0) {
    resampled = LogLoader::loadNavDataset(_resumeAfterPrepare);
  } else {
    NavDataset raw = loadNavs(*amap, _boatid);
    //resampled = downSampleGpsTo1Hz(raw);
    resampled = raw.createMergedChannels(
        std::set<DataCode>{GPS_POS, GPS_SPEED, GPS_BEARING},
        Duration<>::seconds(0.99));

    if (_gpsFilter) {
      resampled = filterNavs(resampled, _gpsFilterSettings);
    }
  }

  if (_savePreparedData.size() != 0) {
    saveDispatcher(_savePreparedData.c_str(), *(resampled.dispatcher()));
  }

  // Note: the grammar does not have access to proper true wind.
  // It has to do its own estimate.
  resampled = resampled.createMergedChannels(
      std::set<DataCode>{AWA, AWS}, Duration<>::seconds(.3));
  std::shared_ptr<HTree> fulltree = _grammar.parse(resampled);

  if (!fulltree) {
    LOG(WARNING) << "grammar parsing failed. No data? boat: " << _boatid;
    return false;
  }

  Calibrator calibrator(_grammar.grammar);
  if (_verboseCalibrator) { calibrator.setVerbose(); }
  std::string boatDatPath = _dstPath.toString() + "/boat.dat";
  std::ofstream boatDatFile(boatDatPath);

  // Calibrate. TODO: use filtered data instead of resampled.
  if (calibrator.calibrate(resampled, fulltree, _boatid)) {
      calibrator.saveCalibration(&boatDatFile);
  } else {
    LOG(WARNING) << "Calibration failed. Using default calib values.";
    calibrator.clear();
  }

  // First simulation pass: adds true wind
  NavDataset simulated = calibrator.simulate(resampled);

  // This choice should be left to the user.
  // TODO: add a per-boat configuration system
  simulated.preferSource(std::set<DataCode>{TWS, TWDIR, TWA, VMG},
                         "Simulated Anemomind estimator");

  if (_saveSimulated.size() > 0) {
    saveDispatcher(_saveSimulated.c_str(), *(simulated.dispatcher()));
  }

  outputTargetSpeedTable(_debug, 
                         fulltree,
                         _grammar.grammar.nodeInfo(),
                         simulated,
                         _vmgSampleSelection,
                         &boatDatFile);

  // write calibration and target speed to disk
  boatDatFile.close();

  // Second simulation path to apply target speed.
  // Todo: simply lookup the target speed instead of recomputing true wind.
  simulated = SimulateBox(boatDatPath, simulated);

  if (_debug) {
    visualizeBoatDat(_dstPath);
  }

  if (_generateTiles) {
    Array<NavDataset> sessions =
      extractAll("Sailing", simulated, _grammar.grammar, fulltree);

    if (!generateAndUploadTiles(_boatid, sessions, &db, _tileParams)) {
      LOG(ERROR) << "generateAndUpload: tile generation failed";
      return false;
    }
  }

  if (_generateChartTiles) {
    if (!uploadChartTiles(simulated, _boatid, _chartTileSettings, &db)) {
      LOG(ERROR) << "Failed to upload chart tiles!";
      return false;
    }
  }

  LOG(INFO) << "Processing time for " << _boatid << ": "
    << (TimeStamp::now() - start).seconds() << " seconds.";
  return true;
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

  _chartTileSettings.dbName = _tileParams.dbName;
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

  if (_generateTiles || _generateChartTiles) {
    if (!mongoConnect(_tileParams.dbHost,
                      _tileParams.dbName,
                      _tileParams.user,
                      _tileParams.passwd,
                      &db)) {
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

  amap.registerOption("--host", "MongoDB hostname").store(&params->dbHost);
  amap.registerOption("--scale", "max scale level").store(&params->maxScale);
  amap.registerOption("--maxpoints",
                      "downsample curves if they have more than <maxpoints> points")
    .store(&params->maxNumNavsPerSubCurve);

  amap.registerOption("--db", "Name of the db, such as 'anemomind' or 'anemomind-dev'")
      .setArgCount(1)
      .store(&params->dbName);

  amap.registerOption("-u", "username for db connection")
      .store(&params->user);

  amap.registerOption("-p", "password for db connection")
      .store(&params->passwd);

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
