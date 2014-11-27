/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PBM.h"
#include <server/common/string.h>
#include <iostream>

#include <fstream>

namespace sail {

BoolMapImage readPBM(std::string filename) {
  std::ifstream file(filename);
  int width = 0;
  int height = 0;
  {
    std::string header, comment, xy;
    std::getline(file, header);
    std::getline(file, comment);
    std::getline(file, xy);
    std::stringstream ssxy;
    ssxy << xy;
    ssxy >> width;
    ssxy >> height;
  }
  BoolMapImage dst(width, height);
  std::string s;
  Array<Vectorize<bool, 1> > dstdata = dst.getStorage();
  int counter = 0;
  while (file.good()) {
    std::getline(file, s);
    int len = s.length();
    for (int i = 0; i < len; i++) {
      dstdata[counter][0] = (s[i] == '0'? false : true);
      counter++;
    }
  }
  assert(counter == width*height);
  return dst;
}

} /* namespace mmm */
