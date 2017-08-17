/*
 * TimeSets.cpp
 *
 *  Created on: 17 Aug 2017
 *      Author: jonas
 */

#include "TimeSets.h"
#include <server/common/logging.h>
#include <server/common/ArrayBuilder.h>

namespace sail {

// What we name things in the Mongo database
static const char* tsCollection = "timesets";
static const char* tsBegin ="begin";
static const char* tsEnd ="end";
static const char* tsType ="type";
static const char* tsBoat ="boat";

std::shared_ptr<mongoc_collection_t> accessTimesets(
    const std::shared_ptr<mongoc_database_t>& db) {
  return getOrCreateCollection(
      db.get(), tsCollection);
}

std::shared_ptr<bson_t> makeTimeSetsInterval(
    const bson_oid_t& boid,
    const std::string& type,
    const Span<TimeStamp>& sp) {
  auto dst = SHARED_MONGO_PTR(bson, bson_new());
  genOid(dst.get());
  BSON_APPEND_OID(dst.get(), tsBoat, &boid); // TODO: is OID the right type to use?
  BSON_APPEND_UTF8(dst.get(), tsType, type.c_str());
  bsonAppend(dst.get(), tsBegin, sp.minv());
  bsonAppend(dst.get(), tsEnd, sp.maxv());
  LOG(INFO) << "Inser this: " << bsonToString(*dst);
  return dst;
}


bool insertTimeSets(
    const std::shared_ptr<mongoc_database_t>& dstDB,
    const std::string& boatId,
    const std::string& label,
    const Array<Span<TimeStamp>>& spans) {
  auto coll = accessTimesets(dstDB);
  if (!coll) {
    LOG(ERROR) << "Cannot insert time sets with label " << label << " for boat"
        << boatId << " because we could not obtain/create a collection";
    return false;
  }
  BulkInserter inserter(coll);
  auto boid = makeOid(boatId);
  for (auto sp: spans) {
    if (!inserter.insert(makeTimeSetsInterval(boid, label, sp))) {
      return false;
    }
  }
  return true;
}

void queryToMongo(
    const TimeSetsQuery& q,
    bson_t* dst) {
  if (!q.boatId.empty()) {
    bsonAppendAsOid(dst, tsBoat, q.boatId);
  }
  if (!q.type.empty()) {
    BSON_APPEND_UTF8(dst, tsType, q.type.c_str());
  }

  // Only pick intervals such that their last time
  // is greater than lower?
  if (q.lower.defined()) {
    BsonSubDocument end(dst, tsEnd);
    bsonAppend(&end, "$gte", q.lower);
  }

  // Only pick intervals such that their first time
  // is before upper?
  if (q.upper.defined()) {
    BsonSubDocument begin(dst, tsBegin);
  }
}

bool removeTimeSets(
    const std::shared_ptr<mongoc_database_t>& db,
    const TimeSetsQuery& q) {
  auto coll = accessTimesets(db);
  if (!coll) {
    LOG(ERROR) << "Cannot remove time sets, because cannot access collection.";
  }
  WrapBson mq;
  queryToMongo(q, &mq);
  auto wc = mongoWriteConcernForLevel(MONGOC_WRITE_CONCERN_W_DEFAULT);
  bson_error_t error;

  if (!mongoc_collection_remove (
      coll.get(),
      MONGOC_REMOVE_NONE,
      &mq,
      wc.get(),
      &error)) {
    LOG(ERROR) << "Removing timesetes failed: " << bsonErrorToString(error);
    return false;
  }

  return true;
}

struct TimeSetBuilder : public bson_iter_t {
  TimeSetBuilder() : bson_iter_t({0}) {}

  std::string type;
  TimeStamp begin, end;

  static TimeSetBuilder* fromIter(const bson_iter_t* x) {
    return reinterpret_cast<TimeSetBuilder*>(
        const_cast<bson_iter_t*>(x));
  }
};



class BsonVisitor {
public:
  std::vector<std::string> path;

  virtual bool visitUtf8(const char *key,
      size_t v_utf8_len,
      const char *v_utf8,
      void *data)
};

struct BsonIterWithVisitor : public bson_iter_t {
  BsonIterWithVisitor(BsonVisitor* v) : visitor(v), bson_iter_t({0}) {}
  BsonVisitor* visitor;

  static BsonVisitor* getVisitor(const bson_iter_t* i) {
    return reinterpret_cast<BsonIterWithVisitor*>(
        const_cast<bson_iter_t*>(i))->visitor;
  }
};

bool visitTimeSetDateTime(
    const bson_iter_t *iter,
    const char *key,
    int64_t msec_since_epoch,
    void *data) {
  auto dst = TimeSetBuilder::fromIter(iter);
  auto time = TimeStamp::fromMilliSecondsSince1970(msec_since_epoch);
  LOG(INFO) << "    --Visit time";
  if (strcmp(tsBegin, key)) {
    dst->begin = time;
  } else if (strcmp(tsEnd, key)) {
    dst->end = time;
  }
  return true;
}

bool visitUtf8(
    const bson_iter_t *iter,
    const char *key,
    size_t v_utf8_len,
    const char *v_utf8,
    void *data) {
  return BsonIterWithVisitor::getVisitor(iter)->visitUtf8(
      key, v_utf8_len, v_utf8, data);
}


void bsonTraverse(BsonVisitor* visitor, bson_t* x) {
  BsonIterWithVisitor iter(visitor);
  bson_visitor_t v = {0};
  v.visit_document = &visitDocument;
  v.visit_utf8 = &visitUtf8;
  v.visit_date_time = &visitDateTime;
}


TimeSetInterval bsonToTimeSetInterval(
    const bson_t& src) {
  TimeSetBuilder iter;
  bson_visitor_t visitor = {0};
  visitor.visit_date_time = &visitTimeSetDateTime;
  visitor.visit_utf8 = &visitTimeSetString;
  int count = 0;
  LOG(INFO) << "The data: " << bsonToString(src);
  if (bson_iter_init (&iter, &src)) {
    LOG(INFO) << "-----Visit them all";
    bson_iter_visit_all (&iter, &visitor, &count);
  }
  return TimeSetInterval{
    iter.type,
    iter.begin.defined() && iter.end.defined()?
        Span<TimeStamp>(iter.begin, iter.end)
        : Span<TimeStamp>()};
}

Array<TimeSetInterval> getTimeSets(
    const std::shared_ptr<mongoc_database_t>& db,
    const TimeSetsQuery& q) {
  auto coll = accessTimesets(db);
  if (!coll) {
    LOG(ERROR) << "Cannot get time sets, because cannot access collection";
    return Array<TimeSetInterval>();
  }
  uint32_t skip = 0;
  uint32_t limit = 0;
  uint32_t batchSize = 0;
  WrapBson mq;
  queryToMongo(q, &mq);
  auto cursor = SHARED_MONGO_PTR(
      mongoc_cursor,
      mongoc_collection_find_with_opts(
          coll.get(), &mq, nullptr, nullptr));
  ArrayBuilder<TimeSetInterval> dst;
  const bson_t* tmp;
  LOG(INFO) << "Try to read from db...";
  while (mongoc_cursor_next(cursor.get(), &tmp)) {
    LOG(INFO) << "Read record.";
    auto x = bsonToTimeSetInterval(*tmp);
    if (x.span.initialized()) {
      dst.add(x);
    }
  }
  return dst.get();
}


} /* namespace sail */
