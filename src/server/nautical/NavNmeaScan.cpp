/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavNmeaScan.h"
#include "NavNmea.h"
#include <server/common/filesystem.h>

namespace sail {

Array<Nav> scanNmeaFolder(Poco::Path p) {
  Array<Poco::Path> files = listFilesRecursively(p, &isNmeaFilePath);
  Array<ParsedNavs> parsedNavs = files.map<ParsedNavs>([&] (Poco::Path p) {return loadNavsFromNmea(p.toString());});
  return flattenAndSort(parsedNavs, ParsedNavs::makeCompleteMask());
}

} /* namespace sail */
