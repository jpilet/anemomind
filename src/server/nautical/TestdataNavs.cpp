/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TestdataNavs.h"
#include <server/nautical/NavNmeaScan.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>

namespace sail {

Array<Nav> getTestdataNavs() {
  Poco::Path p = PathBuilder::makeDirectory(Env::SOURCE_DIR).
       pushDirectory("datasets").
       pushDirectory("Irene").
       get();
   return scanNmeaFolder(p, Nav::debuggingBoatId());
}

} /* namespace sail */
