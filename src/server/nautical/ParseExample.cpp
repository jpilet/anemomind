/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/NavNmea.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/string.h>
#include <Poco/Path.h>
#include <server/nautical/grammars/Grammar001.h>
#include <iostream>

using namespace sail;




namespace {
  void valgrindProvocation() {
    Poco::Path p(Env::SOURCE_DIR);
    p.makeDirectory();
    p.pushDirectory("datasets");
    p.pushDirectory("regates");
    p.pushDirectory("regate_1_dec_07");
    Array<Nav> allnavs = scanNmeaFolder(p);
    Array<Array<Nav> > navs = splitNavsByDuration(allnavs, Duration<double>::minutes(10).seconds());
  }

  void loadAndDispTree() {
    Poco::Path p(Env::SOURCE_DIR);
    p.makeDirectory();
    p.pushDirectory("datasets");
    p.pushDirectory("regates");

    Array<Nav> allnavs = scanNmeaFolder(p);

    Grammar001Settings settings;
    Grammar001 g(settings);

    std::shared_ptr<HTree> tree = g.parse(allnavs);
    tree->disp(&(std::cout), g.nodeInfo(), 0, 2);
    std::cout << EXPR_AND_VAL_AS_STRING(tree->childCount()/2) << std::endl;
    cout << "Done" << endl;
  }
}



int main() {
  loadAndDispTree();
  //valgrindProvocation();
  return 0;
}
