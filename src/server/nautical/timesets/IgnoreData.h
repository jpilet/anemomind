/*
 * IgnoreData.h
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_TIMESETS_IGNOREDATA_H_
#define SERVER_NAUTICAL_TIMESETS_IGNOREDATA_H_

#include <server/nautical/timesets/TimeSets.h>
#include <device/anemobox/Dispatcher.h>

namespace sail {

std::function<
  std::shared_ptr<DispatchData>(std::shared_ptr<DispatchData>)>
    ignoreDispatchData(
        const Array<TimeSetInterval>& x,
        const std::set<std::string>& typesOfInterest);

std::shared_ptr<Dispatcher> ignoreData(
    const std::shared_ptr<Dispatcher>& src,
    const Array<TimeSetInterval>& allIntervals,
    const std::set<std::string>& types);

} /* namespace sail */

#endif /* SERVER_NAUTICAL_TIMESETS_IGNOREDATA_H_ */
