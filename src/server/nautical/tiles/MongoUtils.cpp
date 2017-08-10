
#include <mongoc.h>
#include <server/nautical/tiles/MongoUtils.h>
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

void withTemporaryBsonDocument(const std::function<void(bson_t*)>& op) {
  bson_t obj;
  bson_init(&obj);
  op(&obj);
  bson_destroy(&obj);
}

void insertArrayDocument(
    bson_t* dst,
    const std::function<void(bson_t*)>& op) {
  withBsonSubDocument(dst, getNextIndex(dst).str(), op);
}

void withBsonSubDocument(
    bson_t* parent, const char* key,
    const std::function<void(bson_t*)>& op) {
  bson_t sub;
  BSON_APPEND_DOCUMENT_BEGIN(parent, key, &sub);
  op(&sub);
  bson_append_document_end(parent, &sub);
}

void withBsonSubArray(
    bson_t* parent, const char* key,
    const std::function<void(bson_t*)>& op) {
  bson_t sub;
  BSON_APPEND_ARRAY_BEGIN(parent, key, &sub);
  op(&sub);
  bson_append_array_end(parent, &sub);
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




MongoDBConnection::MongoDBConnection(
    const std::string& host,
    const std::string& dbname,
    const std::string& user,
    const std::string& passwd) {
  // if driver is compiled as C++03, see:
  // https://groups.google.com/forum/#!topic/mongodb-user/-dkp8q9ZEGM
  initializeMongo();

  // URI format:
  // http://mongoc.org/libmongoc/current/mongoc_uri_t.html
  std::string accountString;
  if (!user.empty() && !passwd.empty()) {
    accountString = user + ":" + passwd + "@";
  }

  // TODO: Do we need to specify anything else, such as what authentication
  // is to be used?
  auto full = "mongodb://" + accountString + host + "/" + dbname;
  auto uri = UNIQUE_MONGO_PTR(
      mongoc_uri,
      mongoc_uri_new(full.c_str()));


  LOG(INFO) << "Try to connect to Mongo database '"
      << mongoc_uri_get_string(uri.get()) << "'";

  client = SHARED_MONGO_PTR(mongoc_client,
      mongoc_client_new_from_uri(uri.get()));
  if (!client) {
    LOG(ERROR) << "Failed to connect to "
        << host << ". Are the authentication things correct?";
    return;
  }
  mongoc_client_set_error_api(client.get(), 2);
  db = SHARED_MONGO_PTR(mongoc_database,
      mongoc_client_get_database(client.get(), dbname.c_str()));
}

MongoTableName::MongoTableName(const std::string& db, const std::string& table)
  : _db(db), _table(table) {}
std::string MongoTableName::fullName() const {
  CHECK(!_db.empty());
  return _db + "." + _table;
}

bool BulkInserter::insert(const std::shared_ptr<bson_t>& obj) {
  _toInsert.push_back(obj);
  if (_toInsert.size() > 1000) {
    return finish();
  }
  return _success;
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
  mongoc_bulk_operation_execute(op.get(), &uninitialized_reply, nullptr);
  bson_destroy(&uninitialized_reply);
  return true;
}

bool BulkInserter::finish() {
  if (_toInsert.size() == 0) {
    return _success;
  }
  auto collection = UNIQUE_MONGO_PTR(
      mongoc_collection,
      mongoc_database_get_collection(
          _db.get(), _tableName.c_str()));
  bool ordered = true;
  if (_success) {
    _success = withBulkOperation(
        collection.get(), ordered,
        nullptr, // TODO: <-- What about write concern?
        [this](
            mongoc_bulk_operation_t* op) {
      for (auto x: _toInsert) {
        mongoc_bulk_operation_insert(op, x.get());
      }
    });
  }
  _toInsert.clear();
  return _success;
}


};  // namespace sail

