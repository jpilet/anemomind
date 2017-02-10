/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BOATLOGPROCESSOR_H_
#define BOATLOGPROCESSOR_H_

#include <Poco/Path.h>
#include <server/common/ArgMap.h>
#include <server/nautical/Nav.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/nautical/tiles/ChartTiles.h>
#include <server/nautical/tiles/MongoUtils.h>
#include <server/nautical/tiles/NavTileUploader.h>

namespace sail {

namespace DOM {struct Node;}

enum VmgSampleSelection {
  VMG_SAMPLES_FROM_GRAMMAR,
  VMG_SAMPLES_BLIND
};

struct GrammarRunner {
  WindOrientedGrammarSettings settings;
  WindOrientedGrammar grammar;

  GrammarRunner() : grammar(settings) { }
  std::shared_ptr<HTree> parse(const NavDataset& navs) {
    std::shared_ptr<HTree> fulltree = grammar.parse(navs);
    return fulltree;
  }
};

struct BoatLogProcessor {
  bool process(ArgMap* amap);
  void readArgs(ArgMap* amap);
  bool prepare(ArgMap* amap);
  void infoNavDataset(
      const std::string& info, const NavDataset& ds,
      DOM::Node *dst);

  bool _debug = false;
  Nav::Id _boatid;
  Poco::Path _dstPath;
  std::string _htmlReportName;
  TileGeneratorParameters _tileParams;
  ChartTileSettings _chartTileSettings;
  GpsFilterSettings _gpsFilterSettings;
  GrammarRunner _grammar;
  bool _generateTiles = false;
  bool _generateChartTiles = false;
  VmgSampleSelection _vmgSampleSelection;
  std::string _saveSimulated;
  bool _gpsFilter = false;
  std::string _resumeAfterPrepare;
  std::string _savePreparedData;
  bool _verboseCalibrator = false;
  bool _exploreGrammar = false;
  bool _logGrammar = false;

  mongo::DBClientConnection db;
};

int mainProcessBoatLogs(int argc, const char **argv);


} /* namespace sail */

#endif /* BOATLOGPROCESSOR_H_ */
