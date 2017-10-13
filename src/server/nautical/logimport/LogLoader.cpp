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
#include <server/nautical/logimport/SourceGroup.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/math.h>
#include <server/nautical/GeographicPosition.h>
#include <server/nautical/logimport/Nmea0183Loader.h>
#include <server/nautical/logimport/ProtobufLogLoader.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Transducer.h>
#include <server/common/FrequencyLimiter.h>
#include <server/nautical/BoatSpecificHacks.h>
#include <server/common/DynamicScope.h>
#include <server/common/BundleTransducer.h>
#include <server/common/PositiveMod.h>

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
    return ProtobufLogLoader::load(filename, &_acc);
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

Array<std::string> listFilesToLoad(
    const Poco::Path& name) {
  FileTraverseSettings settings;
  settings.visitDirectories = false;
  settings.visitFiles = true;
  ArrayBuilder<std::string> toLoad;
  traverseDirectory(
      name,
      [&](const Poco::Path &path) {
    std::string ext = toLower(path.getExtension());
    if (ext == "txt" || ext == "csv" || ext == "log") {
      toLoad.add(path.toString());
    } else {
      // Silently ignore files with unknown extensions while scanning
      // the directory
    }
  }, settings);
  return toLoad.get();
}

bool LogLoader::load(const Poco::Path &name) {
  int failCount = 0;
  auto filenames = listFilesToLoad(name);
  for (auto filename: filenames) {
    if (!loadFile(filename)) {
      if (failCount < 12) { // So that we don't flood the log file if there are many files.
        LOG(ERROR) << "Failed to load log file " << filename;
      }
      failCount++;
    }
  }
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

NavDataset loadNavDataset(const std::string& filename) {
  LogLoader loader;
  loader.load(filename);
  return loader.makeNavDataset();
}

LogFileInfo analyzeLogFileData(const std::string& filename) {
  LogLoader loader;
  loader.load(filename);

  auto ds = loader.makeNavDataset();

  if (ds.isDefaultConstructed()) {
    return LogFileInfo();
  } else {
    LogFileInfo dst;
    dst.filename = filename;
    if (!loader.acc().bootCounts.empty()) {
      dst.bootCount = *(loader.acc().bootCounts.begin());
    }

    // Summarize all the timestamps.
    TimestampsVisitor v;
    visitDispatcherChannelsConst(ds.dispatcher().get(), &v);

    auto T
      = trMap([](const std::pair<
          std::pair<DataCode, std::string>, std::vector<TimeStamp>>& x) {
          return x.second;
        })
        |
        trCat<std::vector<TimeStamp>>();

    std::vector<TimeStamp> allTimestamps;
    transduceIntoColl(T, &allTimestamps, v.timestamps);
    if (!allTimestamps.empty()) {
      std::sort(allTimestamps.begin(), allTimestamps.end());
      dst.minTime = allTimestamps.front();
      dst.medianTime = allTimestamps[allTimestamps.size()/2];
      dst.maxTime = allTimestamps.back();
    }

    return dst;
  }
}

bool hasLogFileData(const LogFileInfo& info) {
  return !info.filename.empty();
}

Poco::Path toPath(const std::string& s) {
  return Poco::Path(s);
}

Array<LogFileInfo>
  rotateLogFilesSoThatTheFirstOneStartsAfterTheBiggestGap(
      const Array<LogFileInfo>& info) {
  if (info.size() < 2) {
    return info;
  }

  // Find the max gap
  int n = info.size();
  std::pair<Duration<double>, int> best(1.0_days, 1);
  for (int i = 0; i < n; i++) {
    int next = (i + 1) % n;
    Duration<double> gap = positiveMod<Duration<double>>(
        info[next].medianTime - info[i].medianTime, 1.0_days);
    best = std::max(best, std::make_pair(gap, next));
  }

  // Create the rotate result
  Array<LogFileInfo> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = info[(i + best.second) % n];
  }
  return dst;
}

std::vector<LogFileInfo> listLogFilesForSensei(
    const std::vector<std::string>& searchPaths) {
  auto showProgress = progressNotifier<std::string>(
      [](int count, const std::string& filename) {
    LOG(INFO) << "PRE-parsing log file " << filename;
  });
  auto T0 =
      trMap(&toPath)
      |
      trMap(&listFilesToLoad)
      |
      trCat<Array<std::string>>()
      |
      trVisit(showProgress)
      |
      trMap(&analyzeLogFileData)
      |
      trFilter(&hasLogFileData);

  std::vector<LogFileInfo> summaries0;
  transduceIntoColl(T0, &summaries0, searchPaths);
  std::sort(summaries0.begin(), summaries0.end(),
      LogFileInfo::OrderByBootCountAndFilename());

  auto T1 =
      trBundle([](const LogFileInfo& a, const LogFileInfo& b) {
        return a.bootCountAndFilenameKey().first
            != b.bootCountAndFilenameKey().first;
      })
      |
      trMap(&rotateLogFilesSoThatTheFirstOneStartsAfterTheBiggestGap)
      |
      trCat<Array<LogFileInfo>>();

  std::vector<LogFileInfo> summaries1;
  transduceIntoColl(T1, &summaries1, summaries0);
  return summaries1;
}

NavDataset loadUsingBootCountInsteadOfTime(
    const std::vector<std::string>& searchPaths) {

  auto files = listLogFilesForSensei(searchPaths);
  LogLoader loader;
  auto filename = "/tmp/hackinfo.txt";
  std::ofstream ofile(filename);



  // Activate the hack
  Bind<bool> binding(&(hack::performTimeGuessNow), true);
  for (const auto& file: files) {
    std::cout << "LOAD FILE    " << file.filename << std::endl;
    if (file.bootCount.defined()) {
      std::cout << "boot count: " << file.bootCount.get() << std::endl;
      ofile << "Boot count: " << (file.bootCount.get()) << std::endl;
    }

    auto from = hack::nmea0183TimeGuess;
    loader.load(file.filename);
    ofile << "Loaded file spanning from "
        << from.toString() << " to " << hack::nmea0183TimeGuess.toString()
        << std::endl;
    ofile << "Spanning time of "
        << (hack::nmea0183TimeGuess - from).str() << std::endl;
  }
  std::cout << "Loaded it all, see " << filename << std::endl;
  return loader.makeNavDataset();
}

}
