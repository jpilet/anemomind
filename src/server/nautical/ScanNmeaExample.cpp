/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/common/filesystem.h>
#include <server/common/TestEnv.h>
#include <server/nautical/NavNmea.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>
#include <server/nautical/NavNmeaScan.h>
#include <iostream>

using namespace sail;

namespace {
  void ex001() {
    Poco::Path p = TestEnv().datasets();
    p.pushDirectory("regates");
    Array<Nav> navs = scanNmeaFolder(p);

    std::cout << EXPR_AND_VAL_AS_STRING(navs.size()) << std::endl;
  }


}

int main() {
  ex001();
  return 0;
}



