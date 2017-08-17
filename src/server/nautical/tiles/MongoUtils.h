#ifndef NAUTICAL_TILES_MONGO_UTILS_H
#define NAUTICAL_TILES_MONGO_UTILS_H

#include <server/common/TimeStamp.h>
#include <string>
#include <bson.h>
#include <boost/noncopyable.hpp>
#include <mongoc.h>
#include <server/common/string.h>
#include <array>

template <typename T>
using MongoDestructor = void(*)(T*);

template <typename T>
using UniqueMongoPtr = std::unique_ptr<T, MongoDestructor<T>>;


#define UNIQUE_MONGO_PTR(type, x) UniqueMongoPtr<type##_t>(x, &type##_destroy)
#define SHARED_MONGO_PTR(type, x) std::shared_ptr<type##_t>(x, &type##_destroy)

namespace sail {

// Probably you want to use MONGOC_WRITE_CONCERN_W_DEFAULT with this.
std::shared_ptr<mongoc_write_concern_t> mongoWriteConcernForLevel(
    uint32_t level);

bson_oid_t makeOid(const std::string& s);

bson_t* bsonAppend(bson_t* builder, const char* key, const TimeStamp& value);

struct WrapBson : public bson_t, public boost::noncopyable {
  WrapBson() { bson_init(this); }
  ~WrapBson() { bson_destroy(this); }
};

std::ostream& operator<<(std::ostream& s, const bson_error_t& e);
std::string bsonErrorToString(const bson_error_t& e);
std::ostream& operator<<(std::ostream& s, const bson_t& x);
std::string bsonToString(const bson_t& s);

static constexpr int bsonIndexStringLength = 13;
typedef std::array<char, bsonIndexStringLength> IndexString;

// If you pass in nullptr as 'key' argument to
// BsonSubDocument or BsonSubArray, a key will be generated
// as if you were inserting an element to an array.
// For the sake of readability, you can use the 'nextMongoArrayIndex'
// constant instead of a nullptr.
class BsonSubDocument : public bson_t, public boost::noncopyable {
public:
  BsonSubDocument(bson_t* parent, const char* key);
  ~BsonSubDocument();
private:
  bson_t* _parent;
};

struct BsonSubArray : public bson_t, boost::noncopyable {
  BsonSubArray(bson_t* parent, const char* key);
  ~BsonSubArray();
private:
  IndexString buf;
  bson_t* _parent;
};


// Just to improve readability
// when we want the key to be generated and we are inserting
// into an array.
static const char* nextMongoArrayIndex = nullptr;

const char* keyOrNextIndex(bson_t* dst, const char* key, IndexString* buf);

void bsonAppend(bson_t* dst, const char* key, int32_t value);
void bsonAppend(bson_t* dst, const char* key, int64_t value);
void bsonAppend(bson_t* dst, const char* key, double value);

template <typename Iterator>
void bsonAppendElements(
    bson_t* dst, const char* key,
    Iterator begin, Iterator end) {
  BsonSubArray arr(dst, key);
  size_t counter = 0;
  for (auto i = begin; i != end; i++) {
    IndexString buf;
    const char* s;
    bson_uint32_to_string(counter, &s, buf.data(), bsonIndexStringLength);
    bsonAppend(&arr, s, *i);
    counter++;
  }
}

template <typename Coll>
void bsonAppendCollection(bson_t* dst, const char* key, const Coll& src) {
  bsonAppendElements(dst, key, src.begin(), src.end());
}


struct MongoDBConnection {
  MongoDBConnection() {}
  std::shared_ptr<mongoc_client_t> client;
  std::shared_ptr<mongoc_database_t> db;

  bool defined() const {
    return bool(client) && bool(db);
  }

  MongoDBConnection(const std::string& uri);
};

struct MongoConnectionSettings {
  std::string dbHost = "localhost";
  std::string dbName = "anemomind-dev";
  std::string user;
  std::string passwd;
};

void genOid(bson_t* dst);
void bsonAppendAsOid(bson_t* dst, const char* key, const std::string& s);

std::string makeMongoDBURI(
    const MongoConnectionSettings& passwd);

class MongoTableName {
public:
  MongoTableName() {}
  MongoTableName(const std::string& db, const std::string& table);
  std::string fullName() const;
  const std::string& localName() const {return _table;}
private:
  std::string _db, _table;
};

std::shared_ptr<mongoc_collection_t> getOrCreateCollection(
    mongoc_database_t* db, const char* name);

class BulkInserter : private boost::noncopyable {
 public:
  BulkInserter(
      const std::shared_ptr<mongoc_collection_t>& coll,
      int batchSize = 1000)
    : _collection(coll), _batchSize(batchSize) { }

  ~BulkInserter() { finish(); }

  bool insert(const std::shared_ptr<bson_t>& obj);

  bool finish();
 private:
  bool success() const;
  void fail();
  std::shared_ptr<mongoc_collection_t> _collection;
  std::vector<std::shared_ptr<bson_t>> _toInsert;
  int _batchSize;
};




};  // namespace sail

#endif  // NAUTICAL_TILES_MONGO_UTILS_H
