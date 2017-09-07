/*
 * TimeSets.h
 *
 *  Created on: 17 Aug 2017
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_TIMESETS_TIMESETS_H_
#define SERVER_NAUTICAL_TIMESETS_TIMESETS_H_

#include <server/common/Span.h>
#include <server/common/TimeStamp.h>
#include <server/nautical/tiles/MongoUtils.h>


namespace sail {

// Bulk insertion for common type and boatId.
bool insertTimeSets(
    const std::shared_ptr<mongoc_database_t>& dstDB,
    const std::string& boatId,
    const std::string& type,
    const Array<Span<TimeStamp>>& spans);

bool isKnownTimeSetType(const std::string& s);

struct TimeSetsQuery {
  // Both boatId and type can be undefined.
  std::string boatId;
  std::string type;

  // Any span that overlaps will be retrieved.
  // The TimeStamps can be undefined.
  TimeStamp lower;
  TimeStamp upper;

  // TODO: Time of insertion, so that we can replay the operations
  // in order.
};

struct TimeSetInterval {
  std::string type;
  Span<TimeStamp> span;

  // TODO: std::string data;
};

// This is a list of the different time sets type that
// we support.
struct TimeSetTypes {
  static std::string ignoreButVisualize;
  static std::string ignoreCompletely;
  static std::string merge;
  static std::string split;
};

bool removeTimeSets(
    const std::shared_ptr<mongoc_database_t>& db,
    const TimeSetsQuery& q);

Array<TimeSetInterval> getTimeSets(
    const std::shared_ptr<mongoc_database_t>& db,
    const TimeSetsQuery& q);

} /* namespace sail */

#endif /* SERVER_NAUTICAL_TIMESETS_TIMESETS_H_ */
