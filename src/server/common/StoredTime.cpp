/*
 * StoredTime.cpp
 *
 *  Created on: Jun 3, 2016
 *      Author: jonas
 */

#include <server/common/StoredTime.h>
#include <server/common/logging.h>
#include <fstream>

namespace sail {

bool saveTimeStampToFile(const std::string &filename, TimeStamp time) {
  try {
    std::ofstream file(filename);
    file << time.toMilliSecondsSince1970();
  } catch (const std::exception &e) {
    LOG(ERROR) << "Failed to save timestamp to " << filename;
    return false;
  }
  return true;
}

TimeStamp loadTimeStampFromFile(const std::string &filename) {
  try {
    std::ifstream file(filename);
    if (file.good()) {
      std::string line;
      std::getline(file, line);
      std::int64_t milliSeconds = std::stol(line);
      return TimeStamp::fromMilliSecondsSince1970(milliSeconds);
    }
    LOG(WARNING) << "Failed to parse time from file named '" << filename << "'";
  } catch (const std::exception &e) {
    LOG(WARNING) << "Failed to load time from '" << filename << "' because " << e.what();
  }
  return TimeStamp::makeUndefined();
}

namespace {
  TimeStamp selectDefined(TimeStamp a, TimeStamp b) {
    return a.defined()? a : b;
  }
}

StoredTime::StoredTime(const std::string &filename, TimeStamp otherwise) :
    _filename(filename) {
  if (otherwise.undefined()) {
    LOG(WARNING) << "otherwise should be defined. It should be a valid time in the absence of a stored time.";
  }

  auto currentTime = TimeStamp::now();
  if (currentTime.undefined()) {
    LOG(FATAL) << "Something is wrong with your system clock";
  } else {
    auto lastKnownTime = selectDefined(loadTimeStampFromFile(filename),
            selectDefined(otherwise, currentTime));
    _offset = lastKnownTime - currentTime; // <=> lastKnownTime = _offset + currentTime
  }
  _lastSync = nowNoSync();
  _writeBackPeriod = Duration<double>::minutes(1.0);
}



TimeStamp StoredTime::now() {
  auto currentTime = nowNoSync();
  // Write it back every now and then, in case of a system crash
  if (currentTime - _lastSync > _writeBackPeriod) {
    writeItBack();
    _lastSync = currentTime;
  }
  return currentTime;
}

TimeStamp StoredTime::nowNoSync() const {
  return TimeStamp::now() + _offset;
}

void StoredTime::writeItBack() const {
  if (!saveTimeStampToFile(_filename, nowNoSync())) {
    LOG(ERROR) << "Failed to write back the current time to the file";
  }
}


StoredTime::~StoredTime() {
  writeItBack();
}

} /* namespace sail */
