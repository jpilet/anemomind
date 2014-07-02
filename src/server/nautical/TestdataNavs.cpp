/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TestdataNavs.h"
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>

namespace sail {

namespace {
  Array<Nav> getNavsFromPath(Poco::Path p) {
    return scanNmeaFolder(p, Nav::debuggingBoatId());
  }
}

Array<Nav> getTestdataNavs() {
  Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR).
       pushDirectory("datasets").
       pushDirectory("Irene").
       get();
  return getNavsFromPath(p);
}



Array<Nav> getTestdataNavs(int argc, const char **argv) {
  const char pathPrefix[] = "--navpath";
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == pathPrefix) {
      if (i < argc-1) {
        return getNavsFromPath(Poco::Path(argv[i+1]));
      }
      else { // Obviously, the user provided --navpath at the end of the command line.
             // Maybe we should not return anything here.
        return Array<Nav>();
      }
    }
  }

  // If the user did not provide anything, return the default test navs.
  return getTestdataNavs();
}

} /* namespace sail */
