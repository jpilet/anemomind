#include <server/nautical/logimport/SailmonDbLoader.h>
#include <server/common/logging.h>
#include <third_party/sqlite/sqlite3.h>
#include <server/common/TimeStamp.h>

namespace sail {

template <typename X>
X stringToX(char* arg) {
  std::stringstream ss;
  ss << arg;
  X i = 0;
  ss >> i;
  return i;
}


TimeStamp parseAbsoluteTime(char* arg) {
  return TimeStamp::fromMilliSecondsSince1970(stringToX<int64_t>(arg)*1000);
}

int64_t parseLogTime(char* arg) {
  return stringToX<int64_t>(arg);
}

int localAndAbsoluteTimeCallback(
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

std::vector<LocalAndAbsoluteTimePair> getSailmonTimeCorrectionTable(sqlite3 *db) {
  const char sql[] = "select l1.log_time, l1.value "
      "+ l2.value * 24 * 60 * 60 as time_sec from "
      "LogData as l1, LogData as l2 where l1.log_time"
      " = l2.log_time and l1.rawId=1 and l2.rawId=0 "
      "and l1.sensorId = l2.sensorId order by l1.log_time asc";
  char* errMsg = nullptr;
  std::vector<LocalAndAbsoluteTimePair> dst;
  auto rc = sqlite3_exec(db, sql, &localAndAbsoluteTimeCallback, &dst, &errMsg);
  if (rc != SQLITE_OK ) {
    fprintf(stderr, "Failed to select data\n");
    fprintf(stderr, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
    sqlite3_close(db);
    return std::vector<LocalAndAbsoluteTimePair>();
  }
  return dst;
}


std::string sensorIdToSourceString(char* c) {
  return std::string("sailmonSensorId(") + c + ")";
}


struct Acc {
  std::vector<LocalAndAbsoluteTimePair> timePairs;
  LogAccumulator acc;

  TimeStamp toAbsoluteTime(int64_t i) const {
    return estimateTime(timePairs, i);
  }
};

int gpsQueryCallback(
    void *data, int argc, char **argv,
    char **azColName) {
  auto acc = reinterpret_cast<Acc*>(data);
  CHECK(argc == 4);
  auto sensorId = sensorIdToSourceString(argv[0]);
  auto logTime = stringToX<int64_t>(argv[1]);
  auto lat = stringToX<double>(argv[2]);
  auto lon = stringToX<double>(argv[3]);

  acc->acc._GPS_POSsources[sensorId].push_back({
    acc->toAbsoluteTime(logTime),
    GeographicPosition<double>(
        Angle<double>::degrees(lon),
        Angle<double>::degrees(lat))
  });

  return 0;
}

void accumulateGpsData(
    const std::shared_ptr<sqlite3>& db,
    Acc* dst) {
  const char query[] = "select "
      "a.sensorId, "
      "a.log_time, "
      "a.value as latitude, "
      "b.value as longitude FROM "
      "LogData as a, LogData as b WHERE "
      "a.sensorId = b.sensorId AND a.log_time = b.log_time "
      "AND a.rawId = 4 AND b.rawId = 5";
  char* errMsg = nullptr;
  auto rc = sqlite3_exec(db, query, &gpsQueryCallback, dst, &errMsg);
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

auto logTimeUnit = 0.001_s;

TimeStamp estimateTime(
    const std::vector<LocalAndAbsoluteTimePair>& pairs,
    int logTime) {
  auto closest = findClosest(pairs, logTime);
  return closest.absoluteTime + double(logTime - closest.logTime)*logTimeUnit;
}


std::shared_ptr<sqlite3> openSailmonDb(const std::string& filename) {
  sqlite3 *db = nullptr;
  int rc = sqlite3_open(filename.c_str(), &db);

  if (rc) {
    LOG(ERROR) << "Can't open database: " << filename << ": "
      << sqlite3_errmsg(db);
    return std::shared_ptr<sqlite3>();
  } else {
    return std::shared_ptr<sqlite3>(db, &sqlite3_close);
  }
}

bool sailmonDbLoad(const std::string &filename, LogAccumulator *dst) {
  auto db = openSailmonDb(filename);
  if (!db) {
    return false;
  }
  return true;
}

}  // namespace sail
