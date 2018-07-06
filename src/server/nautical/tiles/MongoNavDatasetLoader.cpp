/*
 * MongoNavDatasetLoader.cpp
 *
 *  Created on: 6 Jul 2018
 *      Author: jonas
 */

#include "MongoNavDatasetLoader.h"
#include <set>
#include <server/common/Optional.h>

namespace sail {

namespace {
  Optional<std::string> readStructuredMessage(const bson_t* doc) {
    bson_iter_t iter;
    bson_iter_t sub_iter;
    if (bson_iter_init (&iter, doc) &&
        bson_iter_find_descendant(
            &iter, "structuredMessage", &sub_iter)) {
      if (BSON_TYPE_UTF8 == bson_iter_type(&sub_iter)) {
        return std::string(bson_iter_utf8(&sub_iter, nullptr));
      } else {
        LOG(ERROR) << "Field 'structuredMessage' exists, but is of wrong type";
      }
    }
    return {};
  }

  TimeStamp readTime(const bson_t* doc) {
    bson_iter_t iter;
    bson_iter_t sub_iter;
    if (bson_iter_init (&iter, doc) &&
        bson_iter_find_descendant(
            &iter, "when", &sub_iter)) {
      if (BSON_TYPE_DATE_TIME == bson_iter_type(&sub_iter)) {
        return TimeStamp::fromMilliSecondsSince1970(
            bson_iter_date_time(&sub_iter));
      } else {
        LOG(ERROR) << "Field 'when' exists, but is of wrong type";
      }
    }
    return TimeStamp();
  }

  bool startsWith(const std::string& msg, const std::string& p) {
    if (msg.size() < p.size()) {
      return false;
    }
    return msg.substr(0, p.size()) == p;
  }
}

NavDataset loadEvents(
    const NavDataset& dst,
    const MongoDBConnection& connection,
    const std::string& boatId) {
  if (!connection.defined()) {
    LOG(ERROR) << "Cannot loadEvents from undefined connection";
    return NavDataset();
  }
  auto collection = UNIQUE_MONGO_PTR(
        mongoc_collection,
        mongoc_database_get_collection(
            connection.db.get(),
            "events"));
  if (!collection) {
    LOG(ERROR) << "Failed to get the 'events' collection";
    return NavDataset();
  }

  WrapBson query;
  bsonAppendAsOid(&query, "boat", boatId);
  {
    BsonSubDocument nonEmptyMsg(&query, "structuredMessage");
    bsonAppend(&nonEmptyMsg, "$exists", true);
    bsonAppend(&nonEmptyMsg, "$ne", "");
    nonEmptyMsg.finalize();
  }
  auto cursor = UNIQUE_MONGO_PTR(
      mongoc_cursor,
      mongoc_collection_find_with_opts(
          collection.get(),
          &query, nullptr, nullptr));

  const bson_t* doc = nullptr;
  std::set<std::string> unknown;
  TimedSampleCollection<BinaryEdge>::TimedVector sessionEdges;
  while (mongoc_cursor_next (cursor.get(), &doc)) {
    auto msg0 = readStructuredMessage(doc);
    auto t = readTime(doc);
    if (msg0.defined() && t.defined()) {
      auto msg = msg0.get();
      if (msg == "New session") {
        sessionEdges.push_back(TimedValue<BinaryEdge>(
            t, BinaryEdge::ToOn));
      } else if (msg == "End session") {
        sessionEdges.push_back(TimedValue<BinaryEdge>(
            t, BinaryEdge::ToOff));
      } else if (startsWith(msg, "Sail:")) {
      } else {
        unknown.insert(msg);
      }
    }
  }
  if (!unknown.empty()) {
    std::stringstream ss;
    ss << "The following structureMessage values were not recognized:";
    for (const auto& msg: unknown) {
      ss << "\n  * '" << msg << "'";
    }
    LOG(WARNING) << ss.str();
  }
  return dst.addChannel<BinaryEdge>(
      USER_DEF_SESSION, "iosApp", sessionEdges);
}


} /* namespace sail */
