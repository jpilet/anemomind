#ifndef NAUTICAL_TILES_MONGO_UTILS_H
#define NAUTICAL_TILES_MONGO_UTILS_H

#include <server/common/TimeStamp.h>
#include <string>
#include <bson.h>
#include <boost/noncopyable.hpp>
#include <mongoc.h>
#include <server/common/string.h>

template <typename T>
using MongoDestructor = void(*)(T*);

template <typename T>
using UniqueMongoPtr = std::unique_ptr<T, MongoDestructor<T>>;


#define UNIQUE_MONGO_PTR(type, x) UniqueMongoPtr<type##_t>(x, &type##_destroy)
#define SHARED_MONGO_PTR(type, x) std::shared_ptr<type##_t>(x, &type##_destroy)

namespace sail {

bson_t* append(bson_t* builder, const char* key, const TimeStamp& value);

void withTemporaryBsonDocument(const std::function<void(bson_t*)>& op);

void withBsonSubDocument(
    bson_t* parent, const char* key,
    const std::function<void(bson_t*)>& op);

void withBsonSubArray(
    bson_t* parent, const char* key,
    const std::function<void(bson_t*)>& op
    /*Should only contain keys "0", "1", ... */);

void bsonAppend(bson_t* dst, const char* key, int32_t value);
void bsonAppend(bson_t* dst, const char* key, int64_t value);
void bsonAppend(bson_t* dst, const char* key, double value);

template <typename Iterator>
void bsonAppendElements(
    bson_t* dst, const char* key,
    Iterator begin, Iterator end) {
  withBsonSubArray(dst, key, [dst, begin, end](bson_t* dst) {
    size_t counter = 0;
    for (auto i = begin; i != end; i++) {
      PositiveAsString<size_t> s(counter);
      bsonAppend(dst, s.str(), *i);
      counter++;
    }
  });
}

template <typename Coll>
void bsonAppendCollection(bson_t* dst, const char* key, const Coll& src) {
  bsonAppendElements(dst, key, src.begin(), src.end());
}


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
      const std::shared_ptr<mongoc_database_t>& db)
    : _tableName(tableName), _db(db),
      _batchSize(batchSize), _success(true) { }

  ~BulkInserter() { finish(); }

  bool insert(const std::shared_ptr<bson_t>& obj);

  bool finish();

  const std::shared_ptr<mongoc_database_t>& db() const { return _db; }
 private:
  std::string _tableName;
  std::shared_ptr<mongoc_database_t> _db;
  std::vector<std::shared_ptr<bson_t>> _toInsert;
  int _batchSize;
  bool _success;
};




};  // namespace sail

#endif  // NAUTICAL_TILES_MONGO_UTILS_H
