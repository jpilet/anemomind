#ifndef ANEMOBOX_LOGTONAV_H
#define ANEMOBOX_LOGTONAV_H

#include <string>

#include <server/common/Array.h>
#include <server/nautical/Nav.h>

namespace sail {

class LogFile;

NavCollection logFileToNavArray(const LogFile& data);
NavCollection logFileToNavArray(const std::string& filename);

}  // namespace sail

#endif  // ANEMOBOX_LOGTONAV_H
