/*
 * StoredTime.h
 *
 *  Created on: Jun 3, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_STOREDTIME_H_
#define SERVER_COMMON_STOREDTIME_H_

#include <server/common/TimeStamp.h>

namespace sail {

bool saveTimeStampToFile(const std::string &filename, TimeStamp time);
TimeStamp loadTimeStampFromFile(const std::string &filename);

class StoredTime {
 public:
  StoredTime(const std::string &file, TimeStamp otherwise);
  ~StoredTime();
  TimeStamp now();
  TimeStamp nowNoSync() const;
 private:
  TimeStamp _lastSync;
  Duration<double> _offset, _writeBackPeriod;
  std::string _filename;
  void writeItBack() const;
};

} /* namespace sail */

#endif /* SERVER_COMMON_STOREDTIME_H_ */
