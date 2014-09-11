/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/WGS84.h>
#include <iostream>
#include <fstream>

using namespace sail;

namespace {
  void outputXYZMat(std::string filename, Array<Nav> navs) {
    std::ofstream file(filename);
    int count = navs.size();
    file.precision(15);
    for (int i = 0; i < count; i++) {
      Length<double> pos[3];
      WGS84<double>::toXYZ(navs[i].geographicPosition(), pos);
      file << pos[0].meters() << " "
          << pos[1].meters() << " " << pos[2].meters() << std::endl;
    }
  }

//  void outputGlobe(std::string filename) {
//    std::ofstream file(filename);
//    int count = 30;
//    LineKM londeg(0, count, 0.0, 360.0);
//    LineKM latdeg(0, count, -180, 180.0);
//
//    file.precision(15);
//    for (int i = 0; i < count; i++) {
//      Length<double> pos[3];
//      WGS84<double>::toXYZ(gpos, pos);
//    }
//  }
}


int main(int argc, const char **argv) {

  std::string outNavsFilename, globeFilename;

  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--output-navs", "Specify filename for output").store(&outNavsFilename);
  amap.registerOption("--output-globe", "Filename");
  amap.setHelpInfo("Program to output nav data to XYZ coordinates");

  if (amap.parseAndHelp(argc, argv)) {
    if (!outNavsFilename.empty()) {
      Array<Nav> navs = getTestdataNavs(amap);
      outputXYZMat(outNavsFilename, navs);
    }
    return 0;
  }

  return -1;
}


