
#include <server/nautical/tiles/MongoUtils.h>

#include <server/common/logging.h>

using namespace mongo;

namespace sail {

BSONObjBuilder& append(BSONObjBuilder& builder, const char* key,
                       const TimeStamp& value) {
  return builder.appendDate(key,
      value.defined()? Date_t(value.toMilliSecondsSince1970()) : Date_t());
}

bool safeMongoOps(std::string what,
                  DBClientConnection *db,
                  std::function<void(DBClientConnection*)> f) {
  try {
    f(db);
    std::string err = db->getLastError();
    if (err != "") {
      LOG(ERROR) << "error while " << what << ": " << err;
      return false;
    }
  } catch (const DBException &e) {
    LOG(ERROR) << "error while " << what << ": " << e.what();
    return false;
  }
  return true;
}

bool mongoConnect(const std::string& host,
                  const std::string& dbname,
                  const std::string& user,
                  const std::string& passwd,
                  DBClientConnection* db) {
  // Can cause segfault
  // if driver is compiled as C++03, see:
  // https://groups.google.com/forum/#!topic/mongodb-user/-dkp8q9ZEGM
  mongo::client::initialize();

  std::string err;
  if (!db->connect(host, err)) {
    LOG(ERROR) << "mongoDB connection failed: " << err;
    return false;
  }

  if (user.size() > 0) {
    if (!db->auth(dbname, user, passwd, err)) {
      LOG(ERROR) << "mongoDB authentication failed: " << err;
      return false;
    }
  }

  return true;
}

bool BulkInserter::insert(const BSONObj& obj) {
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
  bool r = safeMongoOps("inserting tiles in mongoDB",
      _db, [=](DBClientConnection *db) {
    db->insert(_table, _toInsert);
  });
  _toInsert.clear();
  _success = _success && r;
  return _success;
}


};  // namespace sail

