#include <device/Arduino/NMEAStats/test/ScreenRecordingSimulator.h>
#include <server/nautical/tiles/NavTileUploader.h>
#include <server/common/ArgMap.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/nautical/NavNmeaScan.h>

using namespace sail;

// Convenience method to extract the description of a tree.
std::string treeDescription(const shared_ptr<HTree>& tree,
                            const Grammar& grammar) {
  if (!tree) {
    return "";
  }
  return grammar.nodeInfo()[tree->index()].description();
}

// Recursively traverse the tree to find all sub-tree with a given
// description. Extract the corresponding navs.
Array<Array<Nav>> extractAll(std::string description, Array<Nav> rawNavs,
                             const WindOrientedGrammar& grammar,
                             const std::shared_ptr<HTree>& tree) {
  if (!tree) {
    return Array<Array<Nav>>();
  }

  if (description == treeDescription(tree, grammar)) {
    Array<Nav> navSpan = rawNavs.slice(tree->left(), tree->right());
    return Array<Array<Nav>>::args(navSpan);
  }

  ArrayBuilder<Array<Nav>> result;
  for (auto child : tree->children()) {
    Array<Array<Nav>> fromChild = extractAll(description, rawNavs,
                                             grammar, child);
    for (auto navs : fromChild) {
      result.add(navs);
    }
  }
  return result.get();
}

int main(int argc, const char** argv) {
  ArgMap args;
  TileGeneratorParameters params;

  args.setHelpInfo("Generate vector tiles showing a boat trajectory from NMEA logs.");
  args.registerOption("--host", "MongoDB hostname").store(&params.dbHost);
  args.registerOption("--scale", "max scale level").store(&params.maxScale);
  args.registerOption("--maxpoints",
                      "downsample curves if they have more than <maxpoints> points")
    .store(&params.maxNumNavsPerSubCurve);
  args.registerOption("--table", "mongoDB table to put the tiles in.")
    .store(&params.tileTable);

  std::string boatId;
  args.registerOption("--id", "boat id").setRequired().store(&boatId);

  std::string navPath;
  args.registerOption("--navpath", "Path to a folder containing NMEA files.")
    .setRequired().store(&navPath);

  std::string boatDat;
  args.registerOption("--boatDat", "Path to boat.dat").store(&boatDat);

  std::string polarDat;
  args.registerOption("--polarDat", "Path to polar.dat").store(&polarDat);

  if (args.parse(argc, argv) == ArgMap::Error) {
    return 1;
  }

  ScreenRecordingSimulator simulator;
  ScreenRecordingSimulator* simulatorPtr = 0;
  if (boatDat.size() > 0 || polarDat.size() > 0) {
    if (simulator.prepare(boatDat, polarDat)) {
	    simulatorPtr = &simulator;
    }
  }
  Array<Nav> rawNavs = scanNmeaFolderWithSimulator(navPath, boatId, simulatorPtr);

  if (rawNavs.size() == 0) {
    LOG(FATAL) << "No NMEA data in " << navPath;
  }

  WindOrientedGrammarSettings settings;
  WindOrientedGrammar grammar(settings);
  std::shared_ptr<HTree> fulltree = grammar.parse(rawNavs);

  if (!generateAndUploadTiles(
          boatId, extractAll("Sailing", rawNavs, grammar, fulltree), params)) {
    LOG(FATAL) << "When processing: " << navPath;
  }

  LOG(INFO) << "Done.";

  return 0;
}
