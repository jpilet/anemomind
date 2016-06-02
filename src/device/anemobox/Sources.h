#ifndef ANEMOBOX_SOURCES_H
#define ANEMOBOX_SOURCES_H

#include <string>

namespace sail {

bool sourceIsInternal(const std::string& source);
bool sourceIsExternal(const std::string& source);

}  // namespace sail

#endif // ANEMOBOX_SOURCES_H
