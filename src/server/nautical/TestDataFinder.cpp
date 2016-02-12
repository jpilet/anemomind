/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/TestDataFinder.h>
#include <server/common/filesystem.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>


namespace sail {

namespace {
  Array<Poco::Path> datasetPaths{
    PathBuilder::makeDirectory(Env::SOURCE_DIR).pushDirectory("datasets").get(),
    "/home/jonas/data/datasets"
    // TODO: Add more paths...
  };
}

std::string findTestDataPath(std::string localPath) {
  return resolvePath(localPath, datasetPaths).toString();
}

} /* namespace sail */
