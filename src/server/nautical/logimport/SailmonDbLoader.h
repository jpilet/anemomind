#ifndef SERVER_NAUTICAL_LOGIMPORT_SAILMONDBLOADER_H_
#define SERVER_NAUTICAL_LOGIMPORT_SAILMONDBLOADER_H_

#include <string>

namespace sail {

class LogAccumulator;

bool sailmonDbLoad(const std::string &filename, LogAccumulator *dst);

}  // namespace sail

#endif /* SERVER_NAUTICAL_LOGIMPORT_SAILMONDBLOADER_H_ */
