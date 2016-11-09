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

};  // namespace sail

#endif  // NAUTICAL_TILES_MONGO_UTILS_H
