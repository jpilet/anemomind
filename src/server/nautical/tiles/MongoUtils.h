#ifndef NAUTICAL_TILES_MONGO_UTILS_H
#define NAUTICAL_TILES_MONGO_UTILS_H

#include <server/common/TimeStamp.h>
#include <string>

#include <mongo/client/dbclient.h>
#include <mongo/bson/bson-inl.h>

namespace sail {

bool safeMongoOps(std::string what,
                  mongo::DBClientConnection *db,
                  std::function<void(mongo::DBClientConnection*)> f);

mongo::BSONObjBuilder& append(mongo::BSONObjBuilder& builder, const char* key,
                              const TimeStamp& value);


bool mongoConnect(const std::string& host,
                  const std::string& dbname,
                  const std::string& user,
                  const std::string& passwd,
                  mongo::DBClientConnection* db);

class BulkInserter : private boost::noncopyable {
 public:
  BulkInserter(const std::string& table, int batchSize,
               mongo::DBClientConnection* db)
    : _table(table), _db(db), _batchSize(batchSize), _success(true) { }

  ~BulkInserter() { finish(); }

  bool insert(const mongo::BSONObj& obj);

  bool finish();

  mongo::DBClientConnection* db() const { return _db; }

 private:
  std::string _table;
  mongo::DBClientConnection* _db;
  std::vector<mongo::BSONObj> _toInsert;
  int _batchSize;
  bool _success;
};




};  // namespace sail

#endif  // NAUTICAL_TILES_MONGO_UTILS_H
