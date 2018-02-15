
#include <server/nautical/tiles/MongoUtils.h>
#include <mongoc.h>
#include <server/common/logging.h>


void initializeMongo() {
  static bool initialized = false;
  if (!initialized) {
    mongoc_init();
    initialized = true;
  }
}


namespace sail {

bson_oid_t makeOid(const std::string& s) {
  bson_oid_t oid;
  bson_oid_init_from_string(&oid, s.c_str());
  return oid;
}

void genOid(bson_t* dst) {
  bson_oid_t _id;
  bson_oid_init(&_id, nullptr);
  BSON_APPEND_OID(dst, "_id", &_id); //tile.genOID()
}

void bsonAppendAsOid(bson_t* dst, const char* key, const std::string& s) {
  auto oid = makeOid(s);
  BSON_APPEND_OID(dst, key, &oid);
}


std::shared_ptr<mongoc_write_concern_t> mongoWriteConcernForLevel(
    uint32_t level) {
  auto dst = SHARED_MONGO_PTR(
      mongoc_write_concern,
      mongoc_write_concern_new());
  mongoc_write_concern_set_w(dst.get(), level);
  return dst;
}

bson_t* bsonAppend(bson_t* builder, const char* key,
                       const TimeStamp& value) {
  // What to do for undefined times? In the old version, we added a default-constructed Date_t
  // It corresponds to 0 milliseconds
  // https://mongodb.github.io/mongo-cxx-driver/api/legacy-1.1.0/time__support_8h_source.html

  int64_t undefinedValue = 0;
  BSON_APPEND_DATE_TIME(builder, key,
      value.defined()?
          value.toMilliSecondsSince1970()
          : undefinedValue);
  return builder;
}

void bsonAppend(bson_t* dst, const char* key, const std::string& s) {
  auto c = s.c_str();
  BSON_APPEND_UTF8(dst, key, c);
}


std::ostream& operator<<(std::ostream& s, const bson_error_t& e) {
  s << "bson_error_t(domain="
      << e.domain
      << " code=" << e.code
      << " message=" << e.message << ")";
  return s;
}

std::string bsonErrorToString(const bson_error_t& e) {
  std::stringstream ss;
  ss << e;
  return ss.str();
}

std::ostream& operator<<(std::ostream& s, const bson_t& x) {
  auto c = bson_as_json(&x, nullptr);
  s << c;
  bson_free(c);
  return s;
}


std::string bsonToString(const bson_t& x) {
  auto c = bson_as_json(&x, nullptr);
  std::string s(c);
  bson_free(c);
  return s;
}

const char* keyOrNextIndex(bson_t* dst, const char* key, IndexString* buf) {
  if (key == nullptr) {
    const char* at;
    bson_uint32_to_string(
        bson_count_keys(dst),
        &at, buf->data(),
        bsonIndexStringLength);
    return at;
  } else {
    return key;
  }
}

BsonSubDocument::BsonSubDocument(bson_t* parent, const char* key0)
  : _parent(parent) {
  IndexString buf;
  auto key = keyOrNextIndex(parent, key0, &buf);
  BSON_APPEND_DOCUMENT_BEGIN(
      parent,
      key,
      this);
}

BsonSubDocument::~BsonSubDocument() { finalize(); }

void BsonSubDocument::finalize() {
  if (_parent) {
    bson_append_document_end(_parent, this);
    _parent = nullptr;
  }
}

BsonSubArray::BsonSubArray(bson_t* parent, const char* key_)
  : _parent(parent) {
  auto key = keyOrNextIndex(parent, key_, &buf);
  bson_append_array_begin (
      parent,
      key,
      strlen(key),
      this);
}

BsonSubArray::~BsonSubArray() {
  bson_append_array_end(_parent, this);
}

void bsonAppend(bson_t* dst, const char* key, int32_t value) {
  BSON_APPEND_INT32(dst, key, value);
}

void bsonAppend(bson_t* dst, const char* key, int64_t value) {
  BSON_APPEND_INT64(dst, key, value);
}

void bsonAppend(bson_t* dst, const char* key, double value) {
  BSON_APPEND_DOUBLE(dst, key, value);
}




bool MongoDBConnection::connected() const {
  if (!defined()) {
    return false;
  }
  auto cur = UNIQUE_MONGO_PTR(
      mongoc_cursor,
      mongoc_database_find_collections(
          db.get(), nullptr, nullptr));
  return bool(cur);
}

MongoDBConnection::MongoDBConnection(
    const std::shared_ptr<mongoc_uri_t>& uri) {
  CHECK(bool(uri));

  initializeMongo();

  const char* dbname = mongoc_uri_get_database(uri.get());

  auto uris = mongoc_uri_get_string(uri.get());
  LOG(INFO) << "Try to connect to Mongo database '"
      << uris << "'";

  client = SHARED_MONGO_PTR(mongoc_client,
      mongoc_client_new_from_uri(uri.get()));
  if (!client) {
    LOG(ERROR) << "Failed to connect to "
        << uris << ". Are the authentication things correct?";
    return;
  }
  mongoc_client_set_error_api(client.get(), 2);
  db = SHARED_MONGO_PTR(mongoc_database,
      mongoc_client_get_database(client.get(), dbname));
}

MongoTableName::MongoTableName(const std::string& db, const std::string& table)
  : _db(db), _table(table) {}
std::string MongoTableName::fullName() const {
  CHECK(!_db.empty());
  return _db + "." + _table;
}


std::shared_ptr<mongoc_collection_t> getOrCreateCollection(
    mongoc_database_t* db, const char* name) {
  auto coll = SHARED_MONGO_PTR(
      mongoc_collection,
      mongoc_database_get_collection(db, name));
  if (coll) {
    return coll;
  } else {
    bson_error_t e;
    coll = SHARED_MONGO_PTR(
        mongoc_collection,
        mongoc_database_create_collection(
            db, name, nullptr, &e));
    if (!coll) {
      LOG(ERROR) << "Failed to create collection named '" << name << "': "
          << bsonErrorToString(e);
    }
    return coll;
  }
}


bool BulkInserter::insert(const std::shared_ptr<bson_t>& obj) {
  if (!success()) {
    return false;
  }
  _toInsert.push_back(obj);
  if (_toInsert.size() > 1000) {
    return finish();
  }
  return success();
}

bson_t* unwrapBsonPtr(const std::shared_ptr<bson_t>& ptr) {
  return ptr.get();
}



bool withBulkOperation(
    mongoc_collection_t *collection,
    bool ordered,
    const mongoc_write_concern_t *write_concern,
    const std::function<void(mongoc_bulk_operation_t*)>& f) {
  auto op = UNIQUE_MONGO_PTR(
      mongoc_bulk_operation,
      mongoc_collection_create_bulk_operation(
      collection, ordered, write_concern));
  if (!op) {
    LOG(ERROR) << "Failed to create bulk operation";
    return false;
  }
  f(op.get());
  bson_t uninitialized_reply;
  bson_error_t error;
  bool success = mongoc_bulk_operation_execute(op.get(), &uninitialized_reply, &error);
  if (!success) {
    LOG(ERROR) << "With bulk operation failed: " << bsonErrorToString(error);
  }
  bson_destroy(&uninitialized_reply);
  return success;
}

bool BulkInserter::finish() {
  if (_toInsert.size() == 0) {
    return success();
  }
  bool ordered = false;
  if (success()) {
    auto concern = nullptr;

    /*
     * TODO: I'm wondering if you could replace _toInsert
     * with mongoc_bulk_operation_insert, that is if
     * mongoc_bulk_operation_insert() keeps a copy internally.
     * I guess yes, but I'm not sure. Anyway it is not so important.
     *
     * Consider refactoring this.
     *
     */

    if (!withBulkOperation(
        _collection.get(), ordered,
        concern,
        [this](
            mongoc_bulk_operation_t* op) {
      for (auto x: _toInsert) {
        bson_iter_t iter;
        bson_iter_init (&iter, x.get());
        if (bson_iter_find(&iter,"_id")) {
          WrapBson selector;
          bson_append_value(&selector, "_id", 3, bson_iter_value(&iter));

          WrapBson opts;
          bson_append_bool(&opts, "upsert", 6, true);
          WrapBson update;
          bson_append_document(&update, "$set", 4, x.get());
          bson_error_t error;
          if (!mongoc_bulk_operation_update_one_with_opts(
              op,
              &selector,
              &update,
              &opts,
              &error)) {
            LOG(ERROR) << bsonErrorToString(error);
            fail();
            break;
          }
        } else {
          mongoc_bulk_operation_insert(op, x.get());
        }
      }
    })) {
      fail();
    }
  }
  _toInsert.clear();
  return success();
}

bool BulkInserter::success() const {
  return bool(_collection);
}

void BulkInserter::fail() {
  _collection = std::shared_ptr<mongoc_collection_t>();
}

bool bsonVisitorUtf8Method(
    const bson_iter_t *iter,
    const char *key,
    size_t v_utf8_len,
    const char *v_utf8,
    void *data) {
  return static_cast<BsonVisitor*>(data)->visitUtf8(
      key, std::string(v_utf8, v_utf8_len)) == BsonVisitor::Stop;
}

bool bsonVisitorDateTimeMethod(
    const bson_iter_t *iter,
    const char *key,
    int64_t msec_since_epoch,
    void *data) {
  return static_cast<BsonVisitor*>(data)->visitDateTime(
      key, TimeStamp::fromMilliSecondsSince1970(msec_since_epoch))
      == BsonVisitor::Stop;
}

bson_visitor_t BsonVisitor::makeFullVisitor() {
  bson_visitor_t v = {0};
  v.visit_utf8 = &bsonVisitorUtf8Method;
  v.visit_date_time = &bsonVisitorDateTimeMethod;
  return v;
}

void BsonVisitor::visit(const bson_t& bson, const bson_visitor_t& v) {
  bson_iter_t iter;
  if (bson_iter_init (&iter, &bson)) {
    bson_iter_visit_all(&iter, &v, this);
  }
}



};  // namespace sail

