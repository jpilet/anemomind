/*
 * TimeSetsTest.cpp
 *
 *  Created on: 17 Aug 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/tiles/NavTileUploader.h>
#include <server/nautical/tiles/MongoUtils.h>
#include <server/nautical/timesets/TimeSets.h>

using namespace sail;

std::string boatId = "kattskit";
std::string label = "entirelyUnreliableData";

TimeStamp t(double s) {
  return TimeStamp::UTC(2017, 8, 11, 11, 27, 0) + s*1.0_s;
}

TEST(TimeSetsTest, BasicTest) {
  MongoConnectionSettings cs;
  auto db = MongoDBConnection(
          makeMongoDBURI(cs));
  if (!db.defined()) {
    LOG(WARNING) <<
        "This unit test will not run, "
        "because connection to Mongo DB failed. "
        "That doesn't mean that the test failed.";
    return;
  }
  LOG(INFO) << "Successfully connected, unit test continues.";
  EXPECT_TRUE(insertTimeSets(db.db, boatId, label, {
      Span<TimeStamp>(t(0), t(1))
  }));

}
