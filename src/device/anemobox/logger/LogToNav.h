#ifndef ANEMOBOX_LOGTONAV_H
#define ANEMOBOX_LOGTONAV_H

#include <string>

#include <server/common/Array.h>
#include <server/nautical/NavCompatibility.h>

namespace sail {

class LogFile;

NavDataset logFileToNavArray(const LogFile& data);
NavDataset logFileToNavArray(const std::string& filename);

}  // namespace sail

#endif  // ANEMOBOX_LOGTONAV_H
