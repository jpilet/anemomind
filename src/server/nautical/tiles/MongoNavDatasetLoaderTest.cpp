/*
 * MongoNavDatasetLoaderTest.cpp
 *
 *  Created on: 6 Jul 2018
 *      Author: jonas
 */

#include <server/nautical/tiles/MongoNavDatasetLoader.h>
#include <server/nautical/tiles/MongoUtils.h>
#include <gtest/gtest.h>
#include <server/common/logging.h>

using namespace sail;

TEST(MongoNavDatasetLoader, TestIt) {
  auto db = MongoDBConnection(SHARED_MONGO_PTR(
      mongoc_uri,
      mongoc_uri_new(MongoDBConnection::defaultMongoUri())));

  if (!db.defined()) {
    LOG(WARNING) << "The Mongo database does not seem to be running";
    // If the Mongo server is not running, it is not a bug in our code.
    return;
  }
  LOG(INFO) << "Successfully connected";

  std::string boatId = "k4ttsk1t";
  {
    auto collection = UNIQUE_MONGO_PTR(
          mongoc_collection,
          mongoc_database_get_collection(
              db.db.get(),
              "events"));
    if (!collection) {
      LOG(WARNING) << "Failed to obtain events";
      return;
    }

    { // Remove existing docs.
      bson_error_t removeError;
      WrapBson toRemove;
      bsonAppendAsOid(&toRemove, "boat", boatId);
      if (!mongoc_collection_remove (
          collection.get(),
          MONGOC_REMOVE_NONE,
          &toRemove,
          nullptr,
          &removeError)) {
        LOG(WARNING) << "Failed to remove because: " << removeError.message;
      }
    }
    for (int i = 0; i < 9; i++) {
      bson_error_t error;
      WrapBson toInsert;
      bsonAppendAsOid(&toInsert, "boat", boatId);
      bsonAppend(&toInsert, "when", TimeStamp::UTC(2018, 7, 5, 18, 3, 4 + i));
      bsonAppend(&toInsert, "structuredMessage", (i % 2 == 0)?
          "New session" : "End session");
      EXPECT_TRUE(mongoc_collection_insert (
          collection.get(),
          MONGOC_INSERT_NONE,
          &toInsert, nullptr, &error));
    }
  }

  //boatId = "552b806a35ce7cb254dc9515";
  auto dataset = loadEvents(NavDataset(), db, boatId);
  int toOnCount = 0;
  int toOffCount = 0;
  for (auto s: dataset.samples<USER_DEF_SESSION>()) {
    (*(s.value == BinaryEdge::ToOn? &toOnCount : &toOffCount))++;
  }
  EXPECT_EQ(toOnCount, 5);
  EXPECT_EQ(toOffCount, 4);
}


