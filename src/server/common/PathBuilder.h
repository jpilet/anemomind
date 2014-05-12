/*
 *  Created on: 2014-05-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef PATHBUILDER_H_
#define PATHBUILDER_H_

#include <Poco/Path.h>

namespace sail {

/*
 * Easier to use interface to construct Poco paths.
 */
class PathBuilder {
 public:
  static PathBuilder makeDirectory(std::string base);
  PathBuilder pushDirectory(std::string dir);
  const Poco::Path &get() const {return _path;}
 private:
  PathBuilder(const Poco::Path &p) : _path(p) {}

  Poco::Path _path;
};

} /* namespace sail */

#endif /* PATHBUILDER_H_ */
