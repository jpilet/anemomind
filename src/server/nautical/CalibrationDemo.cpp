/*
 *  Created on: 2014-11-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/AutoCalib.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>

using namespace sail;

namespace {
  void ex0() {
    Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets/psaros33_Banque_Sturdza").get();
    Array<Nav> navs = scanNmeaFolder(p, Nav::debuggingBoatId()).slice(55895, 79143);
    FilteredNavData filtered(navs, 500);
    AutoCalib calib;
    AutoCalib::Results results = calib.calibrate(filtered);
    results.disp();
  }

}

int main(int argc, const char **argv) {
  ex0();
  return 0;
}


