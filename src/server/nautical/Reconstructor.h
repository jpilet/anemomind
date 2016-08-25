/*
 * Reconstructor.h
 *
 *  Created on: 25 Aug 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_RECONSTRUCTOR_H_
#define SERVER_NAUTICAL_RECONSTRUCTOR_H_

#include <device/anemobox/Dispatcher.h>

namespace sail {
namespace Reconstructor {

struct CalibDataChunk {
  Array<TimedValue<GeographicPosition<double>>> filteredPositions;
#define MAKE_DATA_MAP(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, Array<TimedValue<TYPE>>> HANDLE;
FOREACH_CHANNEL(MAKE_DATA_MAP)
#undef MAKE_DATA_MAP
};


}
}

#endif /* SERVER_NAUTICAL_RECONSTRUCTOR_H_ */
