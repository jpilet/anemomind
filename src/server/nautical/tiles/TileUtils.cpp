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


void splitMotionsIntoAnglesAndNorms(
    const TimedSampleCollection<HorizontalMotion<double>>
      ::TimedVector &src,
    TimedSampleCollection<Angle<double>>
      ::TimedVector *gpsBearings,
    TimedSampleCollection<Velocity<double>>
      ::TimedVector *gpsSpeeds) {
  int n = src.size();
  for (const auto &m: src) {
    auto a = m.value.angle();
    auto s = m.value.norm();
    gpsSpeeds->push_back(TimedValue<Velocity<double>>(
        m.time, s));
    if (Velocity<double>::knots(0.1) < s && isFinite(a)) {
      gpsBearings->push_back(TimedValue<Angle<double>>(
          m.time, a));
    }
  }
}

template <DataCode code>
std::string makeFilteredGpsName(const NavDataset &src) {
  std::shared_ptr<DispatchData> d(src.activeChannel(code));
  CHECK(d);

  return d->source() + " merged+filtered";
}

}  // namespace

NavDataset filterNavs(
    const NavDataset& navs,
    DOM::Node *dst,
    const GpsFilterSettings& settings) {
  auto results = filterGpsData(navs, dst, settings);
  if (results.empty()) {
    LOG(ERROR) << "GPS filtering failed";
    return NavDataset();
  }

  auto motions = results.motions;
  TimedSampleCollection<Angle<double>>::TimedVector gpsBearings;
  TimedSampleCollection<Velocity<double>>::TimedVector gpsSpeeds;
  splitMotionsIntoAnglesAndNorms(motions,
      &gpsBearings,
      &gpsSpeeds);
  CHECK(motions.size() == gpsSpeeds.size());

  // TODO: See issue https://github.com/jpilet/anemomind/issues/793#issuecomment-239423894.
  // In short, we need to make sure that NavDataset::stripChannel doesn't
  // throw away valid data that has already been merged.
  NavDataset cleanGps = navs
    .addAndSelectChannel<GeographicPosition<double> >(
      GPS_POS,
      makeFilteredGpsName<GPS_POS>(navs),
      results.positions)
    .addAndSelectChannel<Velocity<double> >(
      GPS_SPEED,
      makeFilteredGpsName<GPS_SPEED>(navs),
      gpsSpeeds)
    .addAndSelectChannel<Angle<double> >(
      GPS_BEARING,
      makeFilteredGpsName<GPS_BEARING>(navs),
      gpsBearings);

  return cleanGps;
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

}  // namespace sail
