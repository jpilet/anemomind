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
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <iostream>
#include <server/nautical/HTreeJson.h>
#include <server/common/PathBuilder.h>
#include <Poco/JSON/Stringifier.h>

#include <server/common/Json.impl.h>

using namespace sail;
using namespace sail::NavCompat;



namespace {
  void valgrindProvocation() {
    Poco::Path dataFolder = PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").
        pushDirectory("regates").
        pushDirectory("regate_1_dec_07").get();


    NavDataset allnavs = scanNmeaFolderWithSimulator(dataFolder, Nav::debuggingBoatId());
    Array<NavDataset> navs = splitNavsByDuration(allnavs, Duration<double>::minutes(10));
  }

  void loadAndDispTree() {
    Poco::Path dataFolder = PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").
        pushDirectory("regates").
        pushDirectory("regate_1_dec_07").get();

    cout << "Load navs" << endl;
    NavDataset allnavs = scanNmeaFolderWithSimulator(dataFolder, Nav::debuggingBoatId());
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

    {
      ofstream file(prefix + "_tree.js");
      Poco::JSON::Stringifier::stringify(json::serializeMapped(tree, navs, g.nodeInfo()), file, 0, 0);
    }{
      ofstream file(prefix + "_navs.js");
      Poco::JSON::Stringifier::stringify(json::serialize(makeArray(navs)), file, 0, 0);
    }{
      ofstream file(prefix + "_tree_node_info.js");
      Poco::JSON::Stringifier::stringify(json::serialize(g.nodeInfo()), file, 0, 0);
    }

    cout << "Done" << endl;
  }
}



int main() {
  loadAndDispTree();
  //valgrindProvocation();
  return 0;
}
