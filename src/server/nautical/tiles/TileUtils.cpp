/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <device/anemobox/simulator/SimulateBox.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Functional.h>
#include <server/common/logging.h>
#include <server/nautical/DownsampleGps.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/tiles/NavTileUploader.h>
#include <server/nautical/tiles/TileUtils.h>

namespace sail {

using namespace sail::NavCompat;

namespace {

NavDataset filterNavs(NavDataset navs) {
  if (!isValid(navs.dispatcher().get())) {
    LOG(FATAL) << "Called with invalid dispatcher, please fix the code calling this function";
    return NavDataset();
  }

  auto results = filterGpsData(navs);
  if (results.empty()) {
    LOG(ERROR) << "GPS filtering failed";
    return NavDataset();
  }

  NavDataset cleanGps = navs.replaceChannel<GeographicPosition<double> >(
      GPS_POS,
      navs.dispatcher()->get<GPS_POS>()->source() + " merged+filtered",
      results.getGlobalPositions());

  return cleanGps;
}

}  // namespace

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
Array<NavDataset> extractAll(std::string description, NavDataset rawNavs,
                             const WindOrientedGrammar& grammar,
                             const std::shared_ptr<HTree>& tree) {
  if (!tree) {
    LOG(FATAL) << "No tree";
    return Array<NavDataset>();
  }

  if (description == treeDescription(tree, grammar)) {
    NavDataset navSpan = slice(rawNavs, tree->left(), tree->right());
    return Array<NavDataset>{navSpan};
  }

  ArrayBuilder<NavDataset> result;
  for (auto child : tree->children()) {
    Array<NavDataset> fromChild = extractAll(description, rawNavs,
                                             grammar, child);
    for (auto navs : fromChild) {
      result.add(navs);
    }
  }
  return result.get();
}

Array<NavDataset> filterSessions(const Array<NavDataset>& sessions) {
  return filter(sail::map(sessions, filterNavs).toArray(),
          [](NavDataset ds) {
      if (getNavSize(ds) == 0) {
        LOG(WARNING) << "Omitting dataset with 0 navs";
        return false;
      } else if (!isValid(ds.dispatcher().get())) {
        LOG(WARNING) << "Omitting invalid dataset";
        return false;
      }
      return true;
    });
}

void processTiles(const TileGeneratorParameters &params,
    std::string boatId, std::string navPath,
    std::string boatDat, std::string polarDat) {

    // Downsampling GPS should be avoided. However, the navCompat stuff still uses
    // GPS sampling to iterate, and we need to have a bound on the frequency.
    // TODO: Stop using makeArray(), getNav and other indexed access
    // TODO: Stop downsampling GPS.
    auto rawNavs0 = downSampleGpsTo1Hz(LogLoader::loadNavDataset(navPath));

    CHECK(isValid(rawNavs0.dispatcher().get())) << "The loaded data is invalid, please fix the code";

    LOG(INFO) << "Raw navs";
    LOG(INFO) << rawNavs0;

    auto simulated = SimulateBox(boatDat, rawNavs0);
    if (simulated.isDefaultConstructed()) {
      LOG(WARNING) << "Simulation failed";
      simulated = rawNavs0;
    }

    LOG(INFO) << "Filtered";
    LOG(INFO) << simulated;

    WindOrientedGrammarSettings settings;
    WindOrientedGrammar grammar(settings);

    std::shared_ptr<HTree> fulltree = grammar.parse(simulated);
    auto extracted = extractAll("Sailing", simulated, grammar, fulltree);

    for (const NavDataset& session : extracted) {
      LOG(INFO) << "Session: "
        << getFirst(session).time().toString()
        << ", until " << getLast(session).time().toString();
      LOG(INFO) << session;
    }

    Array<NavDataset> sessions = filterSessions(extracted);

    if (sessions.empty()) {
      LOG(WARNING) << "No sessions";
    }


    if (!generateAndUploadTiles(boatId, sessions, params)) {
      LOG(FATAL) << "When processing: " << navPath;
    }

    LOG(INFO) << "Done.";

  }

}
