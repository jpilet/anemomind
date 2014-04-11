/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavNmeaScan.h"
#include "NavNmea.h"
#include <server/common/filesystem.h>
#include <server/nautical/NavIndexer.h>

namespace sail {

Array<Nav> scanNmeaFolder(Poco::Path p, NavIndexer &indexer) {
  Array<Poco::Path> files = listFilesRecursively(p, &isNmeaFilePath);
  int count = files.size();
  Array<ParsedNavs> parsedNavs(count);
  for (int i = 0; i < count; i++) {
    parsedNavs[i] = loadNavsFromNmea(files[i].toString(), indexer);
  }
  return flattenAndSort(parsedNavs, ParsedNavs::makeCompleteMask());
}

} /* namespace sail */
