/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/tiles/TileUtils.h>
#include <device/anemobox/simulator/SimulateBox.h>
#include <server/nautical/tiles/NavTileUploader.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/GpsFilter.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/logging.h>
#include <server/common/LogStreamable.h>


namespace sail {

using namespace sail::NavCompat;

NavDataset filterNavs(NavDataset navs) {
  if (!isValid(navs.dispatcher().get())) {
    LOG(WARNING) << "Invalid dispatcher, please fix the code";
    return NavDataset();
  }

  GpsFilter::Settings settings;

  ArrayBuilder<Nav> withoutNulls;
  withoutNulls.addIf(makeArray(navs), [=](const Nav &nav) {
    auto pos = nav.geographicPosition();
    return abs(pos.lat().degrees()) > 0.01
      && abs(pos.lon().degrees()) > 0.01;
  });

  auto results = GpsFilter::filter(fromNavs(withoutNulls.get()), settings);

  if (!results.defined()) {
    LOG(WARNING) << "Failed to apply GPS filter";
    return NavDataset();
  }
  auto d = fromNavs(
      makeArray(results.filteredNavs()).slice(results.inlierMask()));

  if (!isValid(d.dispatcher().get())) {
    LOG(WARNING) << "The dataset constructed from navs is not valid";
    return NavDataset();
  }

  return d;
}

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
    LOG(WARNING) << "No tree";
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

void processTiles(const TileGeneratorParameters &params,
    std::string boatId, std::string navPath,
    std::string boatDat, std::string polarDat) {
    auto rawNavs0 = LogLoader::loadNavDataset(navPath);

    if (!isValid(rawNavs0.dispatcher().get())) {
      LOG(WARNING) << "The loaded data is invalid, please fix the code";
      return;
    }

    LOG(INFO) << "Raw navs";
    LOG_STREAMABLE(INFO, rawNavs0);

    auto simulated = SimulateBox(boatDat, rawNavs0);
    if (simulated.isDefaultConstructed()) {
      LOG(WARNING) << "Simulation failed";
      return;
    }

    LOG(INFO) << "Filtered";
    LOG_STREAMABLE(INFO, simulated);


    WindOrientedGrammarSettings settings;
    WindOrientedGrammar grammar(settings);

    std::shared_ptr<HTree> fulltree = grammar.parse(simulated);
    auto extracted = extractAll("Sailing", simulated, grammar, fulltree);

    Array<NavDataset> sessions =
      filter(map(extracted, filterNavs),
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

    if (sessions.empty()) {
      LOG(WARNING) << "No sessions";
    }


    if (!generateAndUploadTiles(boatId, sessions, params)) {
      LOG(FATAL) << "When processing: " << navPath;
    }

    LOG(INFO) << "Done.";

  }

}
