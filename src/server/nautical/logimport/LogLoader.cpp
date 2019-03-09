/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <fstream>
#include <regex>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/String.h>
#include <server/common/CsvParser.h>
#include <server/common/filesystem.h>
#include <server/common/logging.h>
#include <server/common/math.h>
#include <server/nautical/logimport/iwatch.h>
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

bool hasExtension(const std::string& filename, const char* ext) {
  return std::regex_search(filename, std::regex(std::string(ext) + "$",
                                                std::regex_constants::icase));
}

std::string randomStr(int len) {
  std::string s(len, 0);
  for (int i = 0; i < len; ++i) {
    s[i] = 'A' + (rand() % ('Z' - 'A'));
  }
  return s;
}

std::string uncompressFile(const std::string& filename) {
  Poco::Path path(filename);
  std::string ext = path.getExtension();
  std::string command;

  if (ext == "gz") {
    command = GUNZIP_EXE;
  } else if (ext == "bz2") {
    command = BUNZIP2_EXE;
  } else if (ext == "xz") {
    command = UNXZ_EXE;
  } else {
    return "";
  }

  std::string newfile =
    Poco::Path::temp() + "/" + randomStr(6) + '_' + path.getBaseName();
  
  command += " < '" + filename + "' > '" + newfile + "'";

  LOG(INFO) << "Running: " << command;
  if (system(command.c_str()) == 0) {
    return newfile;
  }
  return "";
}


bool LogLoader::loadFile(const std::string &filename) {
  bool r = false;

  std::string newFilename = uncompressFile(filename);
  if (!newFilename.empty()) {
    r = loadFile(newFilename);
    Poco::File(newFilename).remove();
    return r;
  }

  if (hasExtension(filename, "xls")) {
    r = loadCsvFromPipe(std::string("xls2csv -x '") + filename + "'",
                        "Imported from XLS file", &_acc);
  } else if (hasExtension(filename, "vdr")) {
    r = loadCsvFromPipe(std::string("weather4d '") + filename + "'",
                        "Imported from Weather4D VDR", &_acc);
  } else if (hasExtension(filename, "db")) {
    r = sailmonDbLoad(filename, &_acc);
  } else {
    r =
      parseIwatch(filename, &_acc)
      || ProtobufLogLoader::load(filename, &_acc)
      || Nmea0183Loader::loadNmea0183File(filename, &_acc)
      || loadCsv(filename, &_acc)
      || accumulateAstraLogs(filename, &_acc);
  }

  if (!r) {
    LOG(ERROR) << filename << ": file empty or format not recognized.";
  }

  return r;
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

bool LogLoader::acceptFile(const std::string& filename) {
  Poco::Path path(filename);
  std::string ext = toLower(path.getExtension());

  return ext == "txt" || ext == "csv" || ext == "xls" || ext == "vdr"
        || ext == "log" || ext == "db" || ext == "ast"
        || ext == "json" || ext == "";
}

bool LogLoader::load(const Poco::Path &name) {
  FileTraverseSettings settings;
  settings.visitDirectories = false;
  settings.visitFiles = true;
  int failCount = 0;
  traverseDirectory(
      name,
      [&](const Poco::Path &path) {
    std::string filename = path.toString();
    if (acceptFile(filename)) {
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
