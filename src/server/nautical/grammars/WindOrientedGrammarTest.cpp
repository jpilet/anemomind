/*
 *  Created on: 2014-07-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmea.h>
#include <gtest/gtest.h>
#include <server/nautical/grammars/TreeExplorer.h>
#include <server/common/string.h>

using namespace sail;

namespace {
  TimeStamp makeTS(NavCollection navs, int index) {
    return navs[index].time() + (navs[index+1].time() - navs[index].time()).scaled(0.5);
  }

  UserHint makeStartHint(NavCollection navs, int index) {
    return UserHint(UserHint::RACE_START, makeTS(navs, index));
  }

  UserHint makeEndHint(NavCollection navs, int index) {
    return UserHint(UserHint::RACE_END, makeTS(navs, index));
  }

  bool hasNodeWithStart(std::shared_ptr<HTree> tree, int type, int start) {
    if (tree->index() == type) {
      if (start == tree->left()) {
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
      if (end == tree->right()) {
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

TEST(WindOrientedGrammarTest, Hinting) {
  Poco::Path path = PathBuilder::makeDirectory(Env::SOURCE_DIR).pushDirectory("datasets").pushDirectory("Irene").pushDirectory("2007").pushDirectory("regate_1_dec_07").makeFile("IreneLog.txt").get();
  NavCollection navs = loadNavsFromNmea(path.toString(), Nav::debuggingBoatId()).navs();

  // Refers to a position in the seq, assuming it is indexed continuously from 0 to 1.
  double startFrac = 0.3;
  double endFrac = 0.7;

  // For instance, a state with index 'startIndex' should not be in race,
  // and a state with index 'startIndex+1' should be in race.
  int startIndex = startFrac*navs.size();
  int endIndex = endFrac*navs.size();

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
}
