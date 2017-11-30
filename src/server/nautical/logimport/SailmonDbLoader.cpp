#include <server/nautical/logimport/SailmonDbLoader.h>
#include <server/common/logging.h>
#include <third_party/sqlite/sqlite3.h>
#include <server/common/TimeStamp.h>

namespace sail {

struct LocalAndAbsoluteTimePair {
  int logTime = 0;
  TimeStamp absoluteTime;

  bool operator<(const LocalAndAbsoluteTimePair& p) const {
    return logTime < p.logTime;
  }
};

int64_t stringToInt(char* arg) {
  std::stringstream ss;
  ss << arg;
  int64_t i = 0;
  ss >> i;
  return i;
}

TimeStamp parseAbsoluteTime(char* arg) {
  return TimeStamp::fromMilliSecondsSince1970(stringToInt(arg)*1000);
}

int64_t parseLogTime(char* arg) {
  return stringToInt(arg);
}

int callback(
    void *data, int argc, char **argv,
    char **azColName) {
  CHECK(argc == 2);
  LocalAndAbsoluteTimePair p;
  p.logTime = parseLogTime(argv[0]);
  p.absoluteTime = parseAbsoluteTime(argv[1]);
  (reinterpret_cast<
      std::vector<LocalAndAbsoluteTimePair>*>(data))->push_back(p);
  return 0;
}

std::vector<LocalAndAbsoluteTimePair> getTimeTable(sqlite3 *db) {
  const char sql[] = "select l1.log_time, l1.value "
      "+ l2.value * 24 * 60 * 60 as time_sec from "
      "LogData as l1, LogData as l2 where l1.log_time"
      " = l2.log_time and l1.rawId=1 and l2.rawId=0 "
      "and l1.sensorId = l2.sensorId sort by l1.log_time";
  char* errMsg = nullptr;
  std::vector<LocalAndAbsoluteTimePair> dst;
  auto rc = sqlite3_exec(db, sql, &callback, &dst, &errMsg);
  if (rc != SQLITE_OK ) {
    fprintf(stderr, "Failed to select data\n");
    fprintf(stderr, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
    sqlite3_close(db);
    return std::vector<LocalAndAbsoluteTimePair>();
  }
  return dst;
}

LocalAndAbsoluteTimePair findClosest(
    const std::vector<LocalAndAbsoluteTimePair>& pairs,
    int logTime) {
  CHECK(!pairs.empty());
  LocalAndAbsoluteTimePair p;
  p.logTime = logTime;
  auto y = std::lower_bound(pairs.begin(), pairs.end(), p);
  if (y == pairs.end()) {
    return *(pairs.end()-1);
  }
  return *y;
}

bool sailmonDbLoad(const std::string &filename, LogAccumulator *dst) {
  sqlite3 *db = nullptr;
  int rc = sqlite3_open(filename.c_str(), &db);
  bool success = false;

  if (rc) {
    LOG(ERROR) << "Can't open database: " << filename << ": "
      << sqlite3_errmsg(db);
    return false;
  } else {
    // proceed with loading
  }
  sqlite3_close(db);
  return success;
}

}  // namespace sail
