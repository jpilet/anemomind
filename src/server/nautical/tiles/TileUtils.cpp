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
  auto results = filterGpsData(navs);
  if (results.empty()) {
    LOG(ERROR) << "GPS filtering failed";
    return NavDataset();
  }

  // TODO: See issue https://github.com/jpilet/anemomind/issues/793#issuecomment-239423894.
  // In short, we need to make sure that NavDataset::stripChannel doesn't
  // throw away valid data that has already been merged.
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

}  // namespace sail
