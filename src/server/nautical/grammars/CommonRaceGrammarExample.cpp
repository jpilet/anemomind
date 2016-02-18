/*
 *  Created on: 2014-07-02
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/TreeExplorer.h>
#include <server/nautical/grammars/CommonRaceGrammar.h>
#include <server/nautical/TestdataNavs.h>

using namespace sail;

int main(int argc, const char **argv) {
  ArgMap amap;
  registerGetTestdataNavs(amap);
  if (amap.parse(argc, argv) != ArgMap::Error) {
    NavCollection navs = getTestdataNavs(amap);
    CommonRaceGrammarSettings s;
    CommonRaceGrammar g(s);
    exploreTree(g.nodeInfo(), g.parse(navs));
    return 0;
  }
  return -1;
}


