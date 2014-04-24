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
#include <iostream>

using namespace sail;

namespace {
  void ex001() {
    Poco::Path p(Env::SOURCE_DIR);
    p.makeDirectory();
    p.pushDirectory("datasets");
    p.pushDirectory("regates");
    std::cout << EXPR_AND_VAL_AS_STRING(p.toString()) << std::endl;

    Array<Nav> allnavs = scanNmeaFolder(p, Nav::debuggingBoatId());

    //plotNavTimeVsIndex(allnavs);

    std::cout << EXPR_AND_VAL_AS_STRING(allnavs.size()) << std::endl;

    Array<Array<Nav> > navs = splitNavsByDuration(allnavs, Duration<double>::minutes(10).seconds());

    std::cout << EXPR_AND_VAL_AS_STRING(navs.size()) << std::endl;
    dispNavTimeIntervals(allnavs);
  }


}

int main() {
  ex001();
  return 0;
}



