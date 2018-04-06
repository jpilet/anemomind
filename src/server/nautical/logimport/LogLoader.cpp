/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <fstream>
#include <Poco/Path.h>
#include <Poco/String.h>
#include <server/common/CsvParser.h>
#include <server/common/filesystem.h>
#include <server/common/logging.h>
#include <server/common/math.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/logimport/CsvLoader.h>
#include <server/nautical/logimport/SailmonDbLoader.h>
#include <server/nautical/logimport/SourceGroup.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/math.h>
#include <server/nautical/GeographicPosition.h>
#include <server/nautical/logimport/Nmea0183Loader.h>
#include <server/nautical/logimport/ProtobufLogLoader.h>
#include <server/nautical/logimport/astra/AstraLoader.h>

namespace sail {



bool LogLoader::loadFile(const std::string &filename) {
  std::string ext = toLower(Poco::Path(filename).getExtension());
  if (ext == "txt") {
    Nmea0183Loader::loadNmea0183File(filename, &_acc);
    return true;
  } else if (ext == "csv") {
    loadCsv(filename, &_acc);
    return true;
  } else if (ext == "log") {
    return ProtobufLogLoader::load(filename, &_acc)
      || accumulateAstraLogs(filename, &_acc);
  } else if (ext == "db") {
    return sailmonDbLoad(filename, &_acc);
  } else {
    LOG(ERROR) << filename << ": unknown log file extension.";
    return false;
  }
}

void LogLoader::loadNmea0183(std::istream *s) {
  Nmea0183Loader::loadNmea0183Stream(s, &_acc,
      Nmea0183Loader::getDefaultSourceName());
}

bool LogLoader::load(const std::string &name) {
  return load(Poco::Path(name));
}

bool LogLoader::load(const LogFile &data) {
  ProtobufLogLoader::load(data, &_acc);
  return true;
}

bool LogLoader::load(const Poco::Path &name) {
  FileTraverseSettings settings;
  settings.visitDirectories = false;
  settings.visitFiles = true;
  int failCount = 0;
  traverseDirectory(
      name,
      [&](const Poco::Path &path) {
    std::string ext = toLower(path.getExtension());
    if (ext == "txt" || ext == "csv" || ext == "log" || ext == "db") {
      if (!loadFile(path.toString())) {
        if (failCount < 12) { // So that we don't flood the log file if there are many files.
          LOG(ERROR) << "Failed to load log file " << path.toString();
        }
        failCount++;
      }
    } else {
      // Silently ignore files with unknown extensions while scanning
      // the directory
    }
  }, settings);
  if (0 < failCount) {
    LOG(ERROR) << "Failed to load " << failCount << " files when visiting " << name.toString();
  }

  return 0 == failCount;
}

NavDataset LogLoader::loadNavDataset(const std::string &name) {
  LogLoader loader;
  loader.load(name);
  return loader.makeNavDataset();
}

NavDataset LogLoader::loadNavDataset(const Poco::Path &name) {
  LogLoader loader;
  loader.load(name);
  return loader.makeNavDataset();
}


template <typename T>
void insertValues(DataCode code,
    const std::map<std::string, typename TimedSampleCollection<T>::TimedVector> &src,
    Dispatcher *dst) {
  for (auto kv: src) {
    dst->insertValues<T>(code, kv.first, kv.second);
  }
}

void LogLoader::addToDispatcher(Dispatcher *dst) const {

  // Set the priorities first!
  for (auto kv: _acc._sourcePriority) {
    dst->setSourcePriority(kv.first, kv.second);
  }

#define INSERT_VALUES(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    insertValues<TYPE>(HANDLE, _acc._##HANDLE##sources, dst);
    FOREACH_CHANNEL(INSERT_VALUES);
#undef  INSERT_VALUES
}

NavDataset LogLoader::makeNavDataset() const {
  auto d = std::make_shared<Dispatcher>();
  addToDispatcher(d.get());
  return NavDataset(d);
}

}
