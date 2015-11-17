#ifndef NAUTICAL_NAVTONMEA_H
#define NAUTICAL_NAVTONMEA_H

#include <string>
#include <vector>
#include <server/nautical/Nav.h>

namespace sail {

std::string assembleNmeaSentence(const std::vector<std::string>& args);
std::string nmeaRmc(const Nav& nav);

}  // namespace sail

#endif  // NAUTICAL_NAVTONMEA_H
