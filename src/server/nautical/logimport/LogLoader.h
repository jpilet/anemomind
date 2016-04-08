/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef DEVICE_ANEMOBOX_LOGGER_LOGLOADER_H_
#define DEVICE_ANEMOBOX_LOGGER_LOGLOADER_H_

#include <server/nautical/NavDataset.h>

namespace Poco {class Path;}

class ValueSet;

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
  // Load a file.
  bool loadFile(const std::string &filename);

  // Load a file, or all logfiles in a directory and its subdirectories.
  void load(const std::string &name);
  void load(const Poco::Path &name);

  // Conveniency functions when there is just one thing
  // to load.
  static NavDataset loadNavDataset(const std::string &name);
  static NavDataset loadNavDataset(const Poco::Path &name);

  void addToDispatcher(Dispatcher *dst) const;
  NavDataset makeNavDataset() const;

  // These methods are needed by the various parsers in order
  // to populate this object with data.
#define MAKE_ACCESSORS(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    std::map<std::string, TimedSampleCollection<TYPE>::TimedVector> \
    *get##HANDLE##sources() { \
      return &(_##HANDLE##sources); }
  FOREACH_CHANNEL(MAKE_ACCESSORS)
#undef MAKE_ACCESSORS

  void loadNmea0183(std::istream *s);
  void load(const LogFile &data);

 private:
  void loadValueSet(const ValueSet &set);
  void loadTextData(const ValueSet &stream);

#define MAKE_SOURCE_MAP(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, TimedSampleCollection<TYPE>::TimedVector> _##HANDLE##sources;
  FOREACH_CHANNEL(MAKE_SOURCE_MAP)
#undef  MAKE_SOURCE_MAP

  std::map<std::string, int> _sourcePriority;
};

template <DataCode Code>
typename std::map<std::string, typename TimedSampleCollection<typename TypeForCode<Code>::type >::TimedVector>
  *getChannels(LogLoader *loader) {return nullptr;}


#define MAKE_DATACODE_GETTER(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  template <> \
  inline typename std::map<std::string, TimedSampleCollection<TYPE>::TimedVector> \
    *getChannels<HANDLE>(LogLoader *loader) { \
    return loader->get##HANDLE##sources(); \
  }
FOREACH_CHANNEL(MAKE_DATACODE_GETTER)
#undef MAKE_DATACODE_GETTER



}

#endif /* DEVICE_ANEMOBOX_LOGGER_LOGLOADER_H_ */
