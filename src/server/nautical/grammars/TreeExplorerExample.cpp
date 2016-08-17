/*
 *  Created on: 2014-06-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/TreeExplorer.h>

#include <iostream>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/string.h>
#include <server/nautical/NavCompatibility.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/nautical/logimport/LogLoader.h>

using namespace sail;
using namespace sail::NavCompat;

int main(int argc, char *argv[]) {
  WindOrientedGrammarSettings s;
  WindOrientedGrammar g(s);

  Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR).
      pushDirectory("datasets").
      pushDirectory("Irene").
      get();

  if (argc > 1) {
    p = argv[1];
  }

  LogLoader loader;
  loader.load(p.toString());
  auto navs = loader.makeNavDataset();
  std::shared_ptr<HTree> tree = g.parse(navs);
  assert(bool(tree));
  auto infoFun = [&] (std::shared_ptr<HTree> tree) {
      return stringFormat("%.3g seconds", (getNav(navs, tree->right()-1).time() - getNav(navs, tree->left()).time()).seconds());
    };
  exploreTree(g.nodeInfo(), tree, &std::cout, infoFun);
  return 0;
}
