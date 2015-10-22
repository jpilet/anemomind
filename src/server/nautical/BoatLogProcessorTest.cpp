/*
 *  Created on: 2014-05-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/BoatLogProcessor.h>
#include <server/common/PathBuilder.h>
#include <server/common/Env.h>
#include <Poco/File.h>

using namespace sail;

namespace {
  Poco::Path getTempDataPath() {
    Poco::Path srcpath = PathBuilder::makeDirectory(Env::SOURCE_DIR).
      pushDirectory("datasets/Irene/2008/regate_28_mai_08").get();

    PathBuilder tmppath = PathBuilder::makeDirectory(Env::BINARY_DIR).
      pushDirectory("temp");
    Poco::File(tmppath.get()).createDirectory();

    Poco::Path logpath = tmppath.pushDirectory(Nav::debuggingBoatId()).get();
    Poco::File(srcpath).copyTo(logpath.toString());
    return logpath;
  }
}

TEST(BoatLogProcessor, ProcessingTest) {
  Poco::Path srcpath = getTempDataPath();
  EXPECT_TRUE(Poco::File(srcpath).exists());
  EXPECT_TRUE(Poco::File(srcpath).isDirectory());
  processBoatDataFullFolder(false, srcpath);
  PathBuilder output = PathBuilder::makeDirectory(srcpath).pushDirectory("processed");

  const int fileCount = 5;
  std::string files[fileCount] = {"all_navs.js", "all_tree_node_info.js", "all_tree.js", "some_other_file.js", "boat.dat"};
  bool shouldExist[fileCount] = {true, true, true, false, true};

  for (int i = 0; i < fileCount; i++) {
    Poco::Path filename = output.makeFile(files[i]).get();
    EXPECT_TRUE(shouldExist[i] == Poco::File(filename).exists());
  }
  Poco::File(srcpath).remove(true);
}
