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

  // Boat id of Jaisalmer
  std::string boatId = "552b806a35ce7cb254dc9515";

  auto dataset = loadEvents(db, boatId);
  for (auto s: dataset.samples<USER_DEF_SESSION>()) {
    LOG(INFO) << "  Session edge at " << s.time.toIso8601String()
        << " of type " << (s.value == BinaryEdge::ToOn? "ToOn" : "ToOff");
  }
}


