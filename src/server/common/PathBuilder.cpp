/*
 *  Created on: 2014-05-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PathBuilder.h"

namespace sail {

PathBuilder PathBuilder::makeDirectory(std::string base) {
  return PathBuilder::makeDirectory(Poco::Path(base));
}

PathBuilder PathBuilder::makeDirectory(const char *base) {
  return PathBuilder(std::string(base));
}

PathBuilder PathBuilder::makeDirectory(Poco::Path base) {
  Poco::Path p = base;
  p.makeDirectory();
  return PathBuilder(p);
}

PathBuilder PathBuilder::pushDirectory(std::string dir) {
  Poco::Path x = _path;
  x.pushDirectory(dir);
  return PathBuilder(x);
}


PathBuilder PathBuilder::makeFile(std::string filename) {
  Poco::Path p = _path;
  p.setFileName(filename);
  return PathBuilder(p);
}

} /* namespace sail */
