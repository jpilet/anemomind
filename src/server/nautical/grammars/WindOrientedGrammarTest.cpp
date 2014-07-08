/*
 *  Created on: 2014-07-07
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
  TimeStamp makeTS(Array<Nav> navs, int index) {
    return navs[index].time() + (navs[index+1].time() - navs[index].time()).scaled(0.5);
  }

  UserHint makeStartHint(Array<Nav> navs, int index) {
    return UserHint(UserHint::RACE_START, makeTS(navs, index));
  }

  UserHint makeEndHint(Array<Nav> navs, int index) {
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
  Array<Nav> navs = loadNavsFromNmea(path.toString(), Nav::debuggingBoatId()).navs();

  double ds = 0.3;
  double de = 0.7;

  int is = ds*navs.size();
  int ie = de*navs.size();
  UserHint hints[2] = {makeStartHint(navs, is), makeEndHint(navs, ie)};

  WindOrientedGrammarSettings settings;
  WindOrientedGrammar g(settings);
  std::shared_ptr<HTree> tree = g.parse(navs, Array<UserHint>(2, hints));

  // 37 not in race
  // 38     in race
  int s = is + 1;
  int e = ie + 1;

  EXPECT_TRUE(hasNodeWithEnd(tree, 37, s));
  EXPECT_TRUE(hasNodeWithStart(tree, 38, s));
  EXPECT_TRUE(hasNodeWithEnd(tree, 38, e));
  EXPECT_TRUE(hasNodeWithStart(tree, 37, e));
}
