#ifndef NAUTICAL_TILES_MONGO_UTILS_H
#define NAUTICAL_TILES_MONGO_UTILS_H

#include <server/common/TimeStamp.h>
#include <mongoc-client.h>
#include <bson.h>
#include <string>
#include <boost/noncopyable.hpp>

template <typename T>
using UniqueCPtr = std::unique_ptr<T, void(*)(T*)>;


#define UNIQUE_MONGO_PTR(type, x) UniqueCPtr<type##_t>(x, &type##_destroy)
#define MONGO_PTR(type, x) std::shared_ptr<type##_t>(x, &type##_destroy)

namespace sail {

bool safeMongoOps(std::string what,
                  const std::shared_ptr<mongoc_collection_t>& db,
                  std::function<void(std::shared_ptr<mongoc_collection_t>)> f);

bson_t* append(bson_t* builder, const char* key, const TimeStamp& value);

struct MongoDBConnection {
  std::shared_ptr<mongoc_client_t> client;
  std::shared_ptr<mongoc_database_t> db;

  bool defined() const {
    return bool(client) && bool(db);
  }

  MongoDBConnection(
        const std::string& host,
        const std::string& dbname,
        const std::string& user,
        const std::string& passwd);
};


class BulkInserter : private boost::noncopyable {
 public:
  BulkInserter(
      const std::string& tableName, int batchSize,
      const MongoDBConnection& db)
    : _tableName(tableName), _db(db),
      _batchSize(batchSize), _success(true) { }

  ~BulkInserter() { finish(); }

  bool insert(const std::shared_ptr<bson_t>& obj);

  bool finish();

  const MongoDBConnection& db() const { return _db; }
 private:
  std::string _tableName;
  MongoDBConnection _db;
  std::vector<std::shared_ptr<bson_t>> _toInsert;
  int _batchSize;
  bool _success;
};




};  // namespace sail

#endif  // NAUTICAL_TILES_MONGO_UTILS_H
