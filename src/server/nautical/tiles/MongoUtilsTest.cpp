/*
 * MongoUtilsTest.cpp
 *
 *  Created on: 11 Aug 2017
 *      Author: jonas
 */

#include <server/nautical/tiles/MongoUtils.h>
#include <gtest/gtest.h>
#include <server/common/logging.h>

using namespace sail;

TEST(MongoUtilsTest, TestIt) {
  // If this unit test segfaults, it may be an indication of an inconsistency
  // between Mongo headers and libraries.
  auto tile = SHARED_MONGO_PTR(bson, bson_new());
  {
      BsonSubArray curves(tile.get(), "curves");
      for (auto subCurve: {"a", "b", "c"}) {
        BsonSubDocument curve(&curves, nextMongoArrayIndex);
        BSON_APPEND_UTF8(&curve, "curveId", subCurve);
        {
          BsonSubArray pts(&curve, "points");
        }
        curve.finalize();
      }
  }
}


