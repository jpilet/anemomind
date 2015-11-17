/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/common/filesystem.h>
#include <server/nautical/NavNmea.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <iostream>

using namespace sail;


int main() {
  Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR).pushDirectory("datasets").pushDirectory("regates").get();
  std::cout << EXPR_AND_VAL_AS_STRING(p.toString()) << std::endl;
  Array<Nav> allnavs = scanNmeaFolderWithSimulator(p, Nav::debuggingBoatId());
  std::cout << EXPR_AND_VAL_AS_STRING(allnavs.size()) << std::endl;
  Array<Array<Nav> > navs = splitNavsByDuration(allnavs, Duration<double>::minutes(10));
  std::cout << EXPR_AND_VAL_AS_STRING(navs.size()) << std::endl;
  dispNavTimeIntervals(allnavs);
  return 0;
}



