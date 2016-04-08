/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/logimport/LogLoader.h>

using namespace sail;

TEST(ProtobufLogTest, LoadAFile) {
  auto data = LogLoader::loadNavDataset(
      PathBuilder::makeDirectory(Env::SOURCE_DIR)
        .pushDirectory("datasets")
        .pushDirectory("protobuflog").get());
  EXPECT_LT(0, data.samples<AWA>().size());
  EXPECT_LT(0, data.samples<GPS_POS>().size());
  EXPECT_LT(0, data.samples<GPS_SPEED>().size());
  EXPECT_LT(0, data.samples<GPS_BEARING>().size());
}

TEST(ProtobufLogTest, LoadRudderData) {
  auto data = LogLoader::loadNavDataset(
      PathBuilder::makeDirectory(Env::SOURCE_DIR)
        .pushDirectory("datasets")
        .pushDirectory("boat55dc89e6838caff0240960a9_rudder").get());
  EXPECT_LT(12/*sufficiently large*/, data.samples<RUDDER_ANGLE>().size());
}



