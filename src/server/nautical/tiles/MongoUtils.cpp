
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

bool safeMongoOps(std::string what,
                  const std::shared_ptr<mongoc_collection_t>& db,
                  std::function<void(std::shared_ptr<mongoc_collection_t>)> f) {

  // No exceptions to catch, because we don't throw any, and we are using the C API.
  f(db);

  auto error = mongoc_collection_get_last_error(db.get());
  if (error) {
    LOG(WARNING) << "safeMongoOps failed, TODO display error";
    return false;
  } else {
    return true;
  }
}

MongoDBConnection::MongoDBConnection(const std::string& host,
                  const std::string& dbname,
                  const std::string& user,
                  const std::string& passwd) {
  // if driver is compiled as C++03, see:
  // https://groups.google.com/forum/#!topic/mongodb-user/-dkp8q9ZEGM
  initializeMongo();

  auto uri = std::shared_ptr<mongoc_uri_t>(
      mongoc_uri_new(host.c_str()),
      &mongoc_uri_destroy);
  mongoc_uri_set_username(uri.get(), user.c_str());
  mongoc_uri_set_password(uri.get(), passwd.c_str());

  //mongoc_uri_set_database(uri.get(), dbname.c_str());

  client = std::shared_ptr<mongoc_client_t>(
      mongoc_client_new_from_uri(uri.get()),
      &mongoc_client_destroy);
  if (!client) {
    LOG(ERROR) << "Failed to connect to "
        << host << ". Are the authentication things correct?";
    return;
  }
  mongoc_client_set_error_api(client.get(), 2);
  db = std::shared_ptr<mongoc_database_t>(
      mongoc_client_get_database(client.get(), dbname.c_str()),
      &mongoc_database_destroy);
}

bool BulkInserter::insert(const std::shared_ptr<bson_t>& obj) {
  _toInsert.push_back(obj);
  if (_toInsert.size() > 1000) {
    return finish();
  }
  return _success;
}

bool BulkInserter::finish() {
  if (_toInsert.size() == 0) {
    return _success;
  }
  auto collection = std::shared_ptr<mongoc_collection_t>(
      mongoc_database_get_collection(
          _db.db.get(), _tableName.c_str()),
          &mongoc_collection_destroy);

  bool r = safeMongoOps("inserting tiles in mongoDB",
      _db, [=](const std::shared_ptr<mongoc_client_t>& db) {
    db->insert(_tableName, _toInsert);
  });
  _toInsert.clear();
  _success = _success && r;
  return _success;
}


};  // namespace sail

