/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/tiles/TileUtils.h>
#include <device/anemobox/simulator/SimulateBox.h>
#include <server/nautical/tiles/NavTileUploader.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/nautical/GpsFilter.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/logging.h>


namespace sail {

NavCollection filterNavs(NavCollection navs) {
  GpsFilter::Settings settings;

  ArrayBuilder<Nav> withoutNulls;
  withoutNulls.addIf(navs.makeArray(), [=](const Nav &nav) {
    auto pos = nav.geographicPosition();
    return abs(pos.lat().degrees()) > 0.01
      && abs(pos.lon().degrees()) > 0.01;
  });
  auto results = GpsFilter::filter(NavCollection::fromNavs(withoutNulls.get()), settings);
  return NavCollection::fromNavs(
      results.filteredNavs().makeArray().slice(results.inlierMask()));
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
Array<NavCollection> extractAll(std::string description, NavCollection rawNavs,
                             const WindOrientedGrammar& grammar,
                             const std::shared_ptr<HTree>& tree) {
  if (!tree) {
    return Array<NavCollection>();
  }

  if (description == treeDescription(tree, grammar)) {
    NavCollection navSpan = rawNavs.slice(tree->left(), tree->right());
    return Array<NavCollection>{navSpan};
  }

  ArrayBuilder<NavCollection> result;
  for (auto child : tree->children()) {
    Array<NavCollection> fromChild = extractAll(description, rawNavs,
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
    auto rawNavs0 = scanNmeaFolder(navPath, boatId);
    Array<Nav> rawNavs = rawNavs0.makeArray();

    if (boatDat != "") {
      if (SimulateBox(boatDat, &rawNavs)) {
        LOG(INFO) << "Simulated anemobox over " << rawNavs.size() << " navs.";
      } else {
        LOG(WARNING) << "failed to simulate anemobox using " << boatDat;
      }
    }

    if (rawNavs.size() == 0) {
      LOG(FATAL) << "No NMEA data in " << navPath;
    }

    WindOrientedGrammarSettings settings;
    WindOrientedGrammar grammar(settings);
    std::shared_ptr<HTree> fulltree = grammar.parse(rawNavs0);

    Array<NavCollection> sessions =
      map(extractAll("Sailing", rawNavs0, grammar, fulltree), filterNavs).toArray();

    if (!generateAndUploadTiles(boatId, sessions, params)) {
      LOG(FATAL) << "When processing: " << navPath;
    }

    LOG(INFO) << "Done.";

  }

}
