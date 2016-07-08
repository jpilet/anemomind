/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BOATLOGPROCESSOR_H_
#define BOATLOGPROCESSOR_H_

#include <Poco/Path.h>
#include <server/common/ArgMap.h>
#include <server/nautical/Nav.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/nautical/tiles/NavTileUploader.h>

namespace sail {

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

  bool _debug;
  Nav::Id _boatid;
  Poco::Path _dstPath;
  TileGeneratorParameters _tileParams;
  GrammarRunner _grammar;
  bool _generateTiles;
  VmgSampleSelection _vmgSampleSelection;
  std::string _saveSimulated;
};

int mainProcessBoatLogs(int argc, const char **argv);


} /* namespace sail */

#endif /* BOATLOGPROCESSOR_H_ */
