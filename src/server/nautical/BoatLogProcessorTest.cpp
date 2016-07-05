/*
 *  Created on: 2014-05-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>
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

  Poco::Path boatDatPath = output.makeFile("boat.dat").get();
  EXPECT_TRUE(Poco::File(boatDatPath).exists());

  TargetSpeedTable table;
  EXPECT_TRUE(loadTargetSpeedTable(boatDatPath.toString().c_str(), &table));

  EXPECT_NEAR(2, (double)table._upwind[4], 0.5);
  EXPECT_NEAR(-1, (double)table._downwind[4], 0.5);
}
