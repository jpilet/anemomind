/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TestdataNavs.h"
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/common/Span.h>
#include <iostream>
#include <server/common/string.h>
#include <server/common/ArgMap.h>

namespace sail {

namespace {
  Array<Nav> getNavsFromPath(Poco::Path p) {
    return scanNmeaFolder(p, Nav::debuggingBoatId());
  }
}

Poco::Path getDefaultNavPath() {
  Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR).
         pushDirectory("datasets").
         pushDirectory("Irene").
         get();
  return p;
}

Array<Nav> getTestdataNavs() {
  return getNavsFromPath(getDefaultNavPath());
}

namespace {
  Array<Nav> loadAllNavs(int argc, const char **argv) {
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
}

void registerGetTestdataNavs(ArgMap &amap) {
  amap.registerOption("--slice", "Slices the navs").setArgCount(2).unique();
  amap.registerOption("--navpath", "Specifies the path from where to load the arguments")
      .setArgCount(1).setUnique();
}

Array<Nav> getTestdataNavs(ArgMap &amap) {
  Poco::Path p = (amap.optionProvided("--navpath")?
      Poco::Path(amap.optionArgs("--navpath")[0]->value()).makeDirectory()
      : getDefaultNavPath());
  Array<Nav> navs = getNavsFromPath(p);
  if (amap.optionProvided("--slice")) {
    Array<ArgMap::Arg*> args = amap.optionArgs("--slice");
    int from = -1;
    int to = -1;
    if (tryParseInt(args[0]->value(), &from)
        && tryParseInt(args[1]->value(), &to)) {
      if (0 <= from && from <= to && to <= navs.size()) {
        return navs.slice(from, to);
      } else {
        std::cout << "Slice arguments should both be in range 0.." << navs.size() << std::endl;
        return Array<Nav>();
      }
    } else {
      std::cout << "Failed to parse --slice arguments" << std::endl;
      return Array<Nav>();
    }
  }
  return navs;
}


} /* namespace sail */
