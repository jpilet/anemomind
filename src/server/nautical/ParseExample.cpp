/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/NavNmea.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <Poco/Path.h>
#include <server/nautical/grammars/Grammar001.h>
#include <iostream>

using namespace sail;

int main() {
  Poco::Path p(Env::SOURCE_DIR);
  p.makeDirectory();
  p.pushDirectory("datasets");
  p.pushDirectory("regates");
  Array<Nav> allnavs = scanNmeaFolder(p);
  Array<Array<Nav> > navs = splitNavsByDuration(allnavs, Duration<double>::minutes(10).seconds());

  Grammar001Settings settings;
  Grammar001 g(settings);

  std::shared_ptr<HTree> tree = g.parse(allnavs);

  return 0;
}
