/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/logimport/LogLoader.h>
#include <server/common/Env.h>
#include <server/common/string.h>
#include <fstream>
#include <Poco/Path.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <iostream>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavCompatibility.h>

using namespace sail;
using namespace sail::NavCompat;



namespace {
  void valgrindProvocation() {
    Poco::Path dataFolder = PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").
        pushDirectory("regates").
        pushDirectory("regate_1_dec_07").get();

    LogLoader loader;
    loader.load(dataFolder.toString());
    Array<NavDataset> navs = splitNavsByDuration(loader.makeNavDataset(),
        Duration<double>::minutes(10));
  }

  void loadAndDispTree() {
    Poco::Path dataFolder = PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").
        pushDirectory("regates").
        pushDirectory("regate_1_dec_07").get();

    cout << "Load navs" << endl;
    LogLoader loader;
    loader.load(dataFolder.toString());
    NavDataset allnavs = loader.makeNavDataset();
    cout << "loaded" << endl;

    WindOrientedGrammarSettings settings;
    WindOrientedGrammar g(settings);


    std::shared_ptr<HTree> fulltree = g.parse(allnavs); //allnavs.sliceTo(2000));
    fulltree->disp(&(std::cout), g.nodeInfo(), 0, 2);
    std::cout << EXPR_AND_VAL_AS_STRING(fulltree->childCount()/2) << std::endl;

    std::cout << "PARSED" << std::endl;
    std::string prefix(Env::BINARY_DIR);

    // Create a smaller tree with fewer children.
    std::shared_ptr<HTree> tree(new HInner(fulltree->index(), fulltree->child(0)));

    NavDataset navs = slice(allnavs, tree->left(), tree->right());
    cout << "Done" << endl;
  }
}



int main() {
  loadAndDispTree();
  //valgrindProvocation();
  return 0;
}
