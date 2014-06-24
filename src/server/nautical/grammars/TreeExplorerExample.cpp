/*
 *  Created on: 2014-06-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/TreeExplorer.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>

using namespace sail;

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

  Array<Nav> navs = scanNmeaFolder(p, Nav::debuggingBoatId());
  std::shared_ptr<HTree> tree = g.parse(navs);
  assert(bool(tree));
  exploreTree(g.nodeInfo(), tree);
  return 0;
}
