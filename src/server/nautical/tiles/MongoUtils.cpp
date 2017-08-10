
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

bson_t* append(bson_t* builder, const char* key,
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


MongoDBConnection::MongoDBConnection(const std::string& host,
                  const std::string& dbname,
                  const std::string& user,
                  const std::string& passwd) {
  // if driver is compiled as C++03, see:
  // https://groups.google.com/forum/#!topic/mongodb-user/-dkp8q9ZEGM
  initializeMongo();

  auto uri = UNIQUE_MONGO_PTR(mongoc_uri,
      mongoc_uri_new(host.c_str()));
  mongoc_uri_set_username(uri.get(), user.c_str());
  mongoc_uri_set_password(uri.get(), passwd.c_str());

  //mongoc_uri_set_database(uri.get(), dbname.c_str());

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
  if (op) {
    LOG(ERROR) << "Failed to create bulk operation";
    return false;
  }
  f(op.get());
  auto reply = UNIQUE_MONGO_PTR(bson, bson_new());
  mongoc_bulk_operation_execute(op.get(), reply.get(), nullptr);
  return true;
}

bool BulkInserter::finish() {
  if (_toInsert.size() == 0) {
    return _success;
  }
  auto collection = SHARED_MONGO_PTR(mongoc_collection,
      mongoc_database_get_collection(
          _db.db.get(), _tableName.c_str()));
  bool ordered = true;
  if (_success) {
    _success = withBulkOperation(
        collection.get(), ordered,
        nullptr,
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

