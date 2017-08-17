/*
 * TimeSets.h
 *
 *  Created on: 17 Aug 2017
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_TIMESETS_TIMESETS_H_
#define SERVER_NAUTICAL_TIMESETS_TIMESETS_H_

#include <server/nautical/tiles/MongoUtils.h>
#include <server/common/Span.h>

namespace sail {

bool insertTimeSets(
    const std::shared_ptr<mongoc_database_t>& dstDB,
    const std::string& boatId,
    const std::string& label,
    const Array<Span<TimeStamp>>& spans);

} /* namespace sail */

#endif /* SERVER_NAUTICAL_TIMESETS_TIMESETS_H_ */
