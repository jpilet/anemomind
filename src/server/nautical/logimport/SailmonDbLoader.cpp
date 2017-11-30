#include <server/nautical/logimport/SailmonDbLoader.h>

#include <server/common/logging.h>
#include <third_party/sqlite/sqlite3.h>

namespace sail {

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
