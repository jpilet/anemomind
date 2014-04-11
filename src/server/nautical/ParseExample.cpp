/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/NavNmea.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/string.h>
#include <server/common/HierarchyJson.h>
#include <server/nautical/NavJson.h>
#include <fstream>
#include <Poco/Path.h>
#include <server/nautical/grammars/Grammar001.h>
#include <iostream>
#include <server/nautical/NavIndexer.h>
#include <server/nautical/HTreeJson.h>

using namespace sail;




namespace {
  void valgrindProvocation() {
    Poco::Path p(Env::SOURCE_DIR);
    p.makeDirectory();
    p.pushDirectory("datasets");
    p.pushDirectory("regates");
    p.pushDirectory("regate_1_dec_07");
    BoatTimeNavIndexer ind = BoatTimeNavIndexer::makeTestIndexer();
    Array<Nav> allnavs = scanNmeaFolder(p, ind);
    Array<Array<Nav> > navs = splitNavsByDuration(allnavs, Duration<double>::minutes(10).seconds());
  }

  void loadAndDispTree() {
    Poco::Path p(Env::SOURCE_DIR);
    p.makeDirectory();
    p.pushDirectory("datasets");
    p.pushDirectory("regates");

    BoatTimeNavIndexer ind = BoatTimeNavIndexer::makeTestIndexer();

    cout << "Load navs" << endl;
    Array<Nav> allnavs = scanNmeaFolder(p, ind);
    cout << "loaded" << endl;

    Grammar001Settings settings;
    Grammar001 g(settings);


    std::shared_ptr<HTree> fulltree = g.parse(allnavs); //allnavs.sliceTo(2000));
    fulltree->disp(&(std::cout), g.nodeInfo(), 0, 2);
    std::cout << EXPR_AND_VAL_AS_STRING(fulltree->childCount()/2) << std::endl;

    std::cout << "PARSED" << std::endl;
    std::string prefix = "/home/jonas/data/workspace/cpp/anemomind";

    // Create a smaller tree with fewer children.
    std::shared_ptr<HTree> tree(new HInner(fulltree->index(), fulltree->child(0)));

    Array<Nav> navs = allnavs.slice(tree->left(), tree->right());

    {
      ofstream file(prefix + "_tree.js");
      json::serializeMapped(tree, navs, g.nodeInfo())->stringify(file, 0, 0);
    }{
      ofstream file(prefix + "_navs.js");
      json::serialize(navs).stringify(file, 0, 0);
    }{
      ofstream file(prefix + "_tree_node_info.js");
      json::serialize(g.nodeInfo()).stringify(file, 0, 0);
    }

    cout << "Done" << endl;
  }
}



int main() {
  loadAndDispTree();
  //valgrindProvocation();
  return 0;
}
