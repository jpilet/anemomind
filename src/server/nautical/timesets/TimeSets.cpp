/*
 * TimeSets.cpp
 *
 *  Created on: 17 Aug 2017
 *      Author: jonas
 */

#include "TimeSets.h"
#include <server/common/logging.h>

namespace sail {

std::shared_ptr<mongoc_collection_t> accessTimesets(
    const std::shared_ptr<mongoc_database_t>& db) {
  return getOrCreateCollection(
      db.get(), "timesets");
}

bool insertTimeSets(
    const std::shared_ptr<mongoc_database_t>& dstDB,
    const std::string& boatId,
    const std::string& label,
    const Array<Span<TimeStamp>>& spans) {
  auto coll = accessTimesets(dstDB);
  if (!coll) {
    LOG(ERROR) << "Cannot insert time sets with label " << label << " for boat"
        << boatId << " because we could not obtain/create a collection";
    return false;
  }
  return true;
}

} /* namespace sail */
