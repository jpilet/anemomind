#include <server/nautical/logimport/SailmonDbLoader.h>
#include <server/common/logging.h>
#include <third_party/sqlite/sqlite3.h>
#include <server/common/TimeStamp.h>
#include <server/nautical/logimport/LogAccumulator.h>
#include <server/nautical/BoatSpecificHacks.h>

namespace sail {

namespace {

enum SailmonRawIds {
  SM_GPS_DATE = 0, // DAYS SINCE 1970
  SM_GPS_TIME = 1, // SECONDS SINCE START OF DAY
  SM_GPS_STATUS = 2,
  SM_GPS_QUALITY = 3,
  SM_LATITUDE = 4,
  SM_LONGITUDE = 5,
  SM_ALTITUDE = 6,
  SM_MAGNETIC_VARIATION = 7,
  SM_COURSE_OVER_GROUND_TRUE = 8,
  SM_COURSE_OVER_GROUND_MAGNETIC = 9,
  SM_COURSE_OVER_WATER_TRUE = 10,
  SM_COURSE_OVER_WATER_MAGNETIC = 11,
  SM_CROSS_TRACK_ERROR = 12,
  SM_SPEED_OVER_WATER = 13,
  SM_SPEED_OVER_GROUND = 14,
  SM_VELOCITY_MADE_COURSE = 15,
  SM_HEADING_TRUE = 16,
  SM_HEADING_MAGNETIC = 17,
  SM_RATE_OF_TURN = 18,
  SM_VELOCITY_MADE_GOOD = 19,
  SM_RUDDER_ANGLE_STARBOARD = 20,
  SM_RUDDER_ANGLE_PORT = 21,
  SM_HEELING = 22,
  SM_TRIM_FORE_AFT = 23,
  SM_MAST_ANGLE = 24,
  SM_KEEL_ANGLE = 25,
  SM_CANARD_ANGLE = 26,
  SM_TRIM_TAB_ANGLE = 27,
  SM_WIND_SPEED_TRUE = 28,
  SM_WIND_SPEED_APPARENT = 29,
  SM_WIND_ANGLE_TRUE = 30,
  SM_WIND_ANGLE_APPARENT = 31,
  SM_WIND_DIRECTION_TRUE = 32,
  SM_WIND_DIRECTION_MAGNETIC = 33,
  SM_AIR_TEMPERATURE = 34,
  SM_WATER_TEMPERATURE = 35,
  SM_HUMIDITY_RELATIVE = 36,
  SM_BAROMETRIC_PRESSURE = 37,
  SM_DEPTH_BELOW_TRANSDUCER = 38,
  SM_DISTANCE_TRAVELED_TRIP = 39,
  SM_DISTANCE_TRAVELED_TOTAL = 40,
  SM_LOAD_CELL_ADC_VALUE = 41
};

}  // namespace

template <typename X>
X stringToX(const char* arg) {
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

bool sqlQuery(std::shared_ptr<sqlite3> db, const std::string& sql,
              int (*callback)(void*,int,char**,char**), void * ptr) {
  char* errMsg = nullptr;
  auto rc = sqlite3_exec(db.get(), sql.c_str(), callback, ptr, &errMsg);
  if (rc != SQLITE_OK ) {
    LOG(ERROR) << "SQL error: " << errMsg
      << "\nIn query: " << sql;
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

std::vector<LocalAndAbsoluteTimePair> getSailmonTimeCorrectionTable(
    std::shared_ptr<sqlite3> db) {
  const char sql[] = "select l1.log_time, l1.value "
      "+ l2.value * 24 * 60 * 60 as time_sec from "
      "LogData as l1, LogData as l2 where l1.log_time"
      " = l2.log_time and l1.rawId=1 and l2.rawId=0 "
      "and l1.sensorId = l2.sensorId order by l1.log_time asc";
  std::vector<LocalAndAbsoluteTimePair> dst;
  bool success = sqlQuery(db, sql, &localAndAbsoluteTimeCallback, &dst);
  if (!success) {
    return std::vector<LocalAndAbsoluteTimePair>();
  }
  return dst;
}


std::string sensorIdToSourceString(char* c) {
  return std::string("sailmonSensorId(") + c + ")";
}


struct Acc {
  std::vector<LocalAndAbsoluteTimePair> timePairs;
  LogAccumulator* dst = nullptr;

  TimeStamp toAbsoluteTime(int64_t i) const {
    return estimateTime(timePairs, i);
  }

  template <DataCode code>
  void accumulate(
      const std::string& source,
      TimeStamp time,
      typename TypeForCode<code>::type value) {
    (*getChannels<code>(dst))[source].push_back({time, value});
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

  acc->dst->_GPS_POSsources[sensorId].push_back({
    acc->toAbsoluteTime(logTime),
    GeographicPosition<double>(
        Angle<double>::degrees(lon),
        Angle<double>::degrees(lat))
  });

  return 0;
}

bool accumulateGpsData(
    const std::shared_ptr<sqlite3>& db,
    Acc* dst) {
  const char query[] = "select "
      "a.sensorId, "
      "a.log_time, "
      "a.value as latitude, "
      "b.value as longitude FROM "
      "LogData as a, LogData as b WHERE "
      "a.sensorId = b.sensorId AND a.log_time = b.log_time "
      "AND a.rawId = 4 AND b.rawId = 5 order by a.log_time asc";
  char* errMsg = nullptr;
  auto rc = sqlite3_exec(db.get(), query, &gpsQueryCallback, dst, &errMsg);
  if (rc != SQLITE_OK ) {
    LOG(ERROR) << "Failed to select data\n";
    LOG(ERROR) << "SQL error: %s\n", errMsg;
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

template<DataCode Code, class Converter>
int accumulateCallback(
    void *data, int argc, char **argv,
    char **azColName) {
  auto acc = reinterpret_cast<Acc*>(data);
  CHECK(argc == 3);
  std::string source = sensorIdToSourceString(argv[0]);
  auto logTime = stringToX<int64_t>(argv[2]);
  Converter converter;
  auto value = converter(argv[1]);

  acc-> template accumulate<Code>(source,
                                  acc->toAbsoluteTime(logTime),
                                  value);
  return 0;
}

template<DataCode Code, class Converter>
bool accumulateValues(const std::shared_ptr<sqlite3>& db,
                      int rawId,
                      Acc* dst) {
  std::string sql =
    "SELECT sensorId, value, log_time "
    "FROM LogData WHERE rawId = ";
  sql += objectToString(rawId) + ";";

  int (*callback)(void*,int,char**,char**) = accumulateCallback<Code, Converter>;
  return sqlQuery(db, sql, callback, dst);
}

struct AngleConverter {
  Angle<double> operator()(const char* str) {
    return Angle<double>::degrees(stringToX<double>(str));
  }
};

struct LengthConverter {
  Length<double> operator()(const char* str) {
    return Length<double>::meters(stringToX<double>(str));
  }
};

struct SpeedConverter {
  Velocity<double> operator()(const char* str) {
    return Velocity<double>::metersPerSecond(stringToX<double>(str));
  }
};

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
  Acc acc;
  acc.dst = dst;
  acc.timePairs = getSailmonTimeCorrectionTable(db);
  if (acc.timePairs.empty()) {
    return false;
  }
  if (!accumulateGpsData(db, &acc)) {
    LOG(ERROR) << filename << ": no GPS time information";
    return false;
  }

  hack::motionWeight = 10.0;

  accumulateValues<GPS_BEARING, AngleConverter>(db, SM_COURSE_OVER_GROUND_TRUE, &acc);
  accumulateValues<WAT_SPEED, SpeedConverter>(db, SM_SPEED_OVER_WATER, &acc);
  accumulateValues<GPS_SPEED, SpeedConverter>(db, SM_SPEED_OVER_GROUND, &acc);
  accumulateValues<MAG_HEADING, AngleConverter>(db, SM_HEADING_MAGNETIC, &acc);
  accumulateValues<VMG, SpeedConverter>(db, SM_VELOCITY_MADE_GOOD, &acc);
  accumulateValues<RUDDER_ANGLE, AngleConverter>(db, SM_RUDDER_ANGLE_PORT, &acc);
  accumulateValues<AWA, AngleConverter>(db, SM_WIND_ANGLE_APPARENT, &acc);
  accumulateValues<AWS, SpeedConverter>(db, SM_WIND_SPEED_APPARENT, &acc);
  accumulateValues<TWA, AngleConverter>(db, SM_WIND_ANGLE_TRUE, &acc);
  accumulateValues<TWS, SpeedConverter>(db, SM_WIND_SPEED_TRUE, &acc);
  accumulateValues<TWDIR, AngleConverter>(db, SM_WIND_DIRECTION_TRUE, &acc);
  accumulateValues<WAT_DIST, LengthConverter>(db, SM_DISTANCE_TRAVELED_TRIP, &acc);
  return true;
}

}  // namespace sail
