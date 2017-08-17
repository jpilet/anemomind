/*
 * TimeSetsTest.cpp
 *
 *  Created on: 17 Aug 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/tiles/NavTileUploader.h>
#include <server/nautical/tiles/MongoUtils.h>

using namespace sail;

TEST(TimeSetsTest, BasicTest) {
  TileGeneratorParameters p;
  auto db = MongoDBConnection(
          makeMongoDBURI(
              p.dbHost,
              p.dbName,
              p.user,
              p.passwd));

}
