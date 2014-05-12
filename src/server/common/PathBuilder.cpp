/*
 *  Created on: 2014-05-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PathBuilder.h"

namespace sail {

PathBuilder PathBuilder::makeDirectory(std::string base) {
  Poco::Path p(base);
  p.makeDirectory();
  return p;
}

PathBuilder PathBuilder::pushDirectory(std::string dir) {
  Poco::Path x = _path;
  x.pushDirectory(dir);
  return PathBuilder(x);
}

} /* namespace sail */
