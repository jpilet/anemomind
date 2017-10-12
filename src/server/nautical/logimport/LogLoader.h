/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef DEVICE_ANEMOBOX_LOGGER_LOGLOADER_H_
#define DEVICE_ANEMOBOX_LOGGER_LOGLOADER_H_

#include <server/nautical/NavDataset.h>
#include <server/nautical/logimport/LogAccumulator.h>

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
  bool load(const std::string &name);
  bool load(const Poco::Path &name);

  // Conveniency functions when there is just one thing
  // to load.
  static NavDataset loadNavDataset(const std::string &name);
  static NavDataset loadNavDataset(const Poco::Path &name);

  void addToDispatcher(Dispatcher *dst) const;
  NavDataset makeNavDataset() const;

  // These methods are needed by the various parsers in order
  // to populate this object with data.


  void loadNmea0183(std::istream *s);
  bool load(const LogFile &data);

  const LogAccumulator& acc() const {return _acc;}
 private:
  LogAccumulator _acc;
  void loadValueSet(const ValueSet &set);
  void loadTextData(const ValueSet &stream);
};

struct LogFileInfo {
  // TODO: In a more sophisticated setting, we
  // can put other things here too, such as
  // max time, min time, median time, and so on.

  std::string filename;
  Optional<int> bootCount;

  struct OrderByBootCount {
    bool operator()(const LogFileInfo& a, const LogFileInfo& b) const {
      return !a.bootCount.defined()? true
          : (!b.bootCount.defined()? false
              : a.bootCount.get() < b.bootCount.get());
    }
  };
};

std::vector<LogFileInfo> listLogFiles(
    const std::vector<std::string>& searchPaths);

NavDataset loadUsingBootCountInsteadOfTime(
    const std::vector<std::string>& searchPaths);

}

#endif /* DEVICE_ANEMOBOX_LOGGER_LOGLOADER_H_ */
