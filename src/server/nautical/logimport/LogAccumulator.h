/*
 * LogAccumulator.h
 *
 *  Created on: May 27, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_LOGIMPORT_LOGACCUMULATOR_H_
#define SERVER_NAUTICAL_LOGIMPORT_LOGACCUMULATOR_H_

#include <device/anemobox/Dispatcher.h>

namespace sail {

// Helper object to collect all the loaded data
struct LogAccumulator {
  bool verbose = false;

  #define MAKE_ACCESSORS(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
      std::map<std::string, TimedSampleCollection<TYPE>::TimedVector> \
      *get##HANDLE##sources() { \
        return &(_##HANDLE##sources); }
    FOREACH_CHANNEL(MAKE_ACCESSORS)
  #undef MAKE_ACCESSORS

  #define MAKE_SOURCE_MAP(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    std::map<std::string, TimedSampleCollection<TYPE>::TimedVector> _##HANDLE##sources;
    FOREACH_CHANNEL(MAKE_SOURCE_MAP)
  #undef  MAKE_SOURCE_MAP

  std::map<std::string, int> _sourcePriority;
};


template <DataCode Code>
typename std::map<std::string, typename TimedSampleCollection<typename TypeForCode<Code>::type >::TimedVector>
  *getChannels(LogAccumulator *loader) {return nullptr;}


#define MAKE_DATACODE_GETTER(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  template <> \
  inline typename std::map<std::string, TimedSampleCollection<TYPE>::TimedVector> \
    *getChannels<HANDLE>(LogAccumulator *loader) { \
    return loader->get##HANDLE##sources(); \
  }
FOREACH_CHANNEL(MAKE_DATACODE_GETTER)
#undef MAKE_DATACODE_GETTER

} /* namespace sail */

#endif /* SERVER_NAUTICAL_LOGIMPORT_LOGACCUMULATOR_H_ */
