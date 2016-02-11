/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef DEVICE_ANEMOBOX_LOGGER_LOGLOADER_H_
#define DEVICE_ANEMOBOX_LOGGER_LOGLOADER_H_

#include <device/anemobox/Dispatcher.h>

namespace sail {

class LogFile;

/*
 * Helper class to load a series of log files.
 *
 * The reason for this is that we first want to accumulate
 * all the data before constructing the dispatcher, in order
 * to avoid resorting the data everytime we add a log file.
 *
 */
class LogLoader {
 public:
  void load(const LogFile &data);
  void load(const std::string &filename);

  void addToDispatcher(Dispatcher *dst);
 private:

#define MAKE_SOURCE_MAP(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, TimedSampleCollection<TYPE>::TimedVector> _##HANDLE##sources;
  FOREACH_CHANNEL(MAKE_SOURCE_MAP)
#undef  MAKE_SOURCE_MAP

  std::map<std::string, int> _sourcePriority;
};

}

#endif /* DEVICE_ANEMOBOX_LOGGER_LOGLOADER_H_ */
