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

std::string boatId = "abcde";
std::string label = "entirelyUnreliableData";

TimeStamp t(double s) {
  return TimeStamp::UTC(2017, 8, 11, 11, 27, 0) + s*1.0_s;
}

TEST(TimeSetsTest, BasicTest) {
  auto db = MongoDBConnection(SHARED_MONGO_PTR(
      mongoc_uri, mongoc_uri_new(
          MongoDBConnection::defaultMongoUri())));

  if (!db.connected()) {
    LOG(WARNING) <<
        "This unit test will not run, "
        "because connection to Mongo DB failed. "
        "That doesn't mean that the test failed.";
    return;
  }
  LOG(INFO) << "Successfully connected, unit test continues.";
  for (int i = 0; i < 3; i++) {
    {
      TimeSetsQuery q;
      q.boatId = boatId;
      if (!removeTimeSets(db.db, q)) {
        LOG(WARNING) << "Something might be wrong with the connection";
        return;
      }
    }{
      LOG(INFO) << "Insert time sets";
      EXPECT_TRUE(insertTimeSets(db.db, boatId, label, {
          Span<TimeStamp>(t(0), t(1)),
          Span<TimeStamp>(t(2), t(4)),
          Span<TimeStamp>(t(5), t(9)),
          Span<TimeStamp>(t(10), t(11))
      }));
    }
  }{
    TimeSetsQuery q;
    q.boatId = boatId;
    auto results = getTimeSets(db.db, q);
    EXPECT_EQ(results.size(), 4);
  }{
    TimeSetsQuery q;
    q.boatId = "ccc";
    EXPECT_TRUE(removeTimeSets(db.db, q));
  }{
    TimeSetsQuery q;
    q.boatId = boatId;
    auto results = getTimeSets(db.db, q);
    EXPECT_EQ(results.size(), 4);
  }{
    TimeSetsQuery q;
    q.boatId = "ccc";
    auto results = getTimeSets(db.db, q);
    EXPECT_EQ(results.size(), 0);
  }{
    TimeSetsQuery q;
    q.boatId = boatId;
    auto results = getTimeSets(db.db, q);
    EXPECT_EQ(results.size(), 4);
  }{
    TimeSetsQuery q;
    q.boatId = boatId;
    q.lower = t(9.5);
    auto results = getTimeSets(db.db, q);
    EXPECT_EQ(results.size(), 1);
    auto r = results[0];
    EXPECT_EQ(r.span.minv(), t(10));
    EXPECT_EQ(r.span.maxv(), t(11));
    EXPECT_EQ(r.type, label);
  }{
    TimeSetsQuery q;
    q.boatId = boatId;
    q.upper = t(1.5);
    auto results = getTimeSets(db.db, q);
    EXPECT_EQ(results.size(), 1);
    auto r = results[0];
    EXPECT_EQ(r.span.minv(), t(0));
    EXPECT_EQ(r.span.maxv(), t(1));
    EXPECT_EQ(r.type, label);
  }{
    TimeSetsQuery q;
    q.boatId = boatId;
    q.lower = t(6);
    q.upper = t(8);
    auto results = getTimeSets(db.db, q);
    EXPECT_EQ(results.size(), 1);
    auto r = results[0];
    EXPECT_EQ(r.span.minv(), t(5));
    EXPECT_EQ(r.span.maxv(), t(9));
    EXPECT_EQ(r.type, label);
  }

}
