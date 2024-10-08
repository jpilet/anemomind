/*
 *  Created on: 2014-07-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/WindOrientedGrammar.h>

#include <gtest/gtest.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/string.h>
#include <server/nautical/NavCompatibility.h>
#include <server/nautical/grammars/TreeExplorer.h>
#include <server/nautical/logimport/LogLoader.h>

using namespace sail;
using namespace sail::NavCompat;

namespace {
  TimeStamp makeTS(NavDataset navs, int index) {
    return getNav(navs, index).time() + (getNav(navs, index+1).time() - getNav(navs, index).time()).scaled(0.5);
  }

  UserHint makeStartHint(NavDataset navs, int index) {
    return UserHint(UserHint::RACE_START, makeTS(navs, index));
  }

  UserHint makeEndHint(NavDataset navs, int index) {
    return UserHint(UserHint::RACE_END, makeTS(navs, index));
  }


  // We cannot always expect it to behave perfectly like an array...
  bool almostEquals(int a, int b) {
    return std::abs(a - b) <= 1;
  }

  bool hasNodeWithStart(std::shared_ptr<HTree> tree, int type, int start) {
    if (tree->index() == type) {
      if (almostEquals(start, tree->left())) {
        return true;
      }
    }

    for (auto c : tree->children()) {
      if (hasNodeWithStart(c, type, start)) {
        return true;
      }
    }
    return false;
  }

  bool hasNodeWithEnd(std::shared_ptr<HTree> tree, int type, int end) {
    if (tree->index() == type) {
      if (almostEquals(end, tree->right())) {
        return true;
      }
    }

    for (auto c : tree->children()) {
      if (hasNodeWithEnd(c, type, end)) {
        return true;
      }
    }
    return false;
  }
}

TEST(WindOrientedGrammarTest, SoftStateCost) {
  //double computeMinorStateCost(double twaDegs, int minorState);
  EXPECT_NEAR(computeMinorStateCost(0.0, 0), 0.0, 1.0e-6);
  EXPECT_NEAR(computeMinorStateCost(120, 1), 0.0, 1.0e-6);
  EXPECT_NEAR(computeMinorStateCost(150, 1), 1.0, 1.0e-6);
  EXPECT_NEAR(computeMinorStateCost(135, 1), 0.5, 1.0e-6);
  EXPECT_NEAR(computeMinorStateCost(15, 5), 0.5, 1.0e-6);
  EXPECT_NEAR(computeMinorStateCost(15 + 720, 5), 0.5, 1.0e-6);
  EXPECT_NEAR(computeMinorStateCost(15 - 720, 5), 0.5, 1.0e-6);
}

TEST(WindOrientedGrammarTest, Hinting) {
  Poco::Path path = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("Irene")
    .pushDirectory("2007")
    .pushDirectory("regate_1_dec_07")
    .makeFile("IreneLog.txt").get();

  auto navs = LogLoader::loadNavDataset(path);

/*
 * TODO:
 *
 * The numbers in this code got obsolete after rewriting the NMEA0183 coding.
 * The reason for this is that in the rewritten code, we also handle the GLL
 * sentence which also contains GPS positions (in addition to RMC). And since
 * we sample Nav's at GPS positions, we get more Nav's than we otherwise would.
 *
 *

  // Refers to a position in the seq, assuming it is indexed continuously from 0 to 1.
  double startFrac = 0.3;
  double endFrac = 0.7;

  // For instance, a state with index 'startIndex' should not be in race,
  // and a state with index 'startIndex+1' should be in race.
  int startIndex = startFrac*getNavSize(navs);
  int endIndex = endFrac*getNavSize(navs);

  UserHint hints[2] = {makeStartHint(navs, startIndex), makeEndHint(navs, endIndex)};

  WindOrientedGrammarSettings settings;
  WindOrientedGrammar g(settings);
  std::shared_ptr<HTree> tree = g.parse(navs, Array<UserHint>(2, hints));

  // Indices to where the separation takes place.
  int startBound = startIndex + 1;
  int endBound = endIndex + 1;

  int notInRace = g.hierarchy().getNodeByName("Not in race").index();
  int inRace = g.hierarchy().getNodeByName("In race").index();

  EXPECT_TRUE(hasNodeWithEnd(tree, notInRace, startBound));
  EXPECT_TRUE(hasNodeWithStart(tree, inRace, startBound));
  EXPECT_TRUE(hasNodeWithEnd(tree, inRace, endBound));
  EXPECT_TRUE(hasNodeWithStart(tree, notInRace, endBound));

  */
}
