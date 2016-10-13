/*
 * illustrate.cpp
 *
 *  Created on: 13 Oct 2016
 *      Author: jonas
 */

#include <server/common/ArgMap.h>
#include <server/common/TimeStamp.h>
#include <server/nautical/DownsampleGps.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/BoatLogProcessor.h>
#include <server/nautical/tiles/TileUtils.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/calib/Calibrator.h>

using namespace sail;

struct Setup {
  std::string path;
  std::string fromStr, toStr;
  std::string prefix = PathBuilder::makeDirectory(
      Env::BINARY_DIR).makeFile("illustrate").get().toString();

  TimeStamp from() const {
    return TimeStamp::parse(fromStr);
  }

  TimeStamp to() const {
    return TimeStamp::parse(toStr);
  }
};

NavDataset loadNavs(const std::string &path) {
  LogLoader loader;
  loader.load(path);
  return loader.makeNavDataset();
}

bool makeIllustrations(const Setup &setup) {
    BoatLogProcessor proc;

    NavDataset raw = loadNavs(setup.path)
        .fitBounds();

    raw = raw.slice(earliest(setup.from(), raw.lowerBound()),
                    latest(setup.to(), raw.upperBound()));

    auto resampled = downSampleGpsTo1Hz(raw);
    resampled = filterNavs(resampled, proc._gpsFilterSettings);

    std::shared_ptr<HTree> fulltree = proc._grammar.parse(resampled);

    if (!fulltree) {
      LOG(WARNING) << "grammar parsing failed. No data?";
      return false;
    }

    Calibrator calibrator(proc._grammar.grammar);
    if (proc._verboseCalibrator) { calibrator.setVerbose(); }
    std::string boatDatPath = setup.prefix + "/boat.dat";
    std::ofstream boatDatFile(boatDatPath);

    // Calibrate. TODO: use filtered data instead of resampled.
    if (calibrator.calibrate(resampled, fulltree, "illustrate")) {
        calibrator.saveCalibration(&boatDatFile);
    } else {
      LOG(WARNING) << "Calibration failed. Using default calib values.";
      calibrator.clear();
    }

    // First simulation pass: adds true wind
    NavDataset simulated = calibrator.simulate(resampled);

    /*
  Why this is needed:
  Whenever the NavDataset::samples<...> method is called, a merge is performed
  for that datacode using all the data of the underlying dispatcher
  (not limited in time). Several NavDatasets will be produced by in the following
  code by slicing up the full NavDataset. Before this slicing takes place,
  we want to merge the data, so that it doesn't have to be merged for every
  slice that produce. This saves us a lot of memory. If we decide to refactor
  this code some time, we should think carefully how we want to do the merging.
     */
    simulated.mergeAll();

    /*outputTargetSpeedTable(_debug,
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
    return true;*/
    return true;
}

int main(int argc, const char **argv) {

  Setup setup;
  ArgMap amap;
  amap.registerOption("--path", "Path to dataset")
      .store(&(setup.path))
      .required();
  amap.registerOption("--from", "From date YYYY-MM-DD").store(&(setup.toStr));
  amap.registerOption("--to", "To date YYYY-MM-DD").store(&(setup.fromStr));
  amap.registerOption("--prefix", "Where output should be put")
      .store(&(setup.prefix));

  auto status = amap.parse(argc, argv);
  switch (status) {
  case ArgMap::Done:
    return 0;
  case ArgMap::Continue:
    return makeIllustrations(setup)? 0 : -2;
  case ArgMap::Error:
    return -1;
  };
  return 0;
}
