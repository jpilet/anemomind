/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "EnvUtil.h"
#include <server/common/PathBuilder.h>
#include <server/common/Env.h>

namespace sail {

Poco::Path getDatasetPath(std::string subFolderName) {
  return PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory(subFolderName).get();
}

}
