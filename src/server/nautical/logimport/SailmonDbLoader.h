#ifndef SERVER_NAUTICAL_LOGIMPORT_SAILMONDBLOADER_H_
#define SERVER_NAUTICAL_LOGIMPORT_SAILMONDBLOADER_H_

#include <string>
#include <memory>
#include <vector>
#include <server/common/TimeStamp.h>

struct sqlite3;

namespace sail {

class LogAccumulator;

struct LocalAndAbsoluteTimePair {
  int logTime = 0;
  TimeStamp absoluteTime;

  bool operator<(const LocalAndAbsoluteTimePair& p) const {
    return logTime < p.logTime;
  }
};

std::vector<LocalAndAbsoluteTimePair> getSailmonTimeCorrectionTable(sqlite3 *db);
std::shared_ptr<sqlite3> openSailmonDb(const std::string& filename);
TimeStamp estimateTime(
    const std::vector<LocalAndAbsoluteTimePair>& pairs,
    int logTime);
bool sailmonDbLoad(const std::string &filename, LogAccumulator *dst);

}  // namespace sail

#endif /* SERVER_NAUTICAL_LOGIMPORT_SAILMONDBLOADER_H_ */
