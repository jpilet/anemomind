/*
 *  Created on: 2014-03-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TestEnv.h"
#include <assert.h>

namespace sail {


TestEnv::TestEnv() {
  Poco::Path thisFilePath(__FILE__);
  assert(thisFilePath.getFileName() == "TestEnv.cpp");

  _root = thisFilePath.makeDirectory();
  _root.popDirectory();
  _root.popDirectory();
  _root.popDirectory();
  _root.popDirectory();

  _datasets = _root;
  _datasets.pushDirectory("datasets");
}

} /* namespace sail */
