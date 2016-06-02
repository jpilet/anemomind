#ifndef ANEMOBOX_SOURCES_H
#define ANEMOBOX_SOURCES_H

#include <string>

namespace sail {

enum SourceOrigin {
  // sensors inside the anemobox and calculations done inside the anemobox
  ANEMOBOX,

  // Post-processing on anemolab
  POST_PROCESS,

  // External instruments
  EXTERNAL,

  // Uknown origin.
  UNKNOWN
};
        
SourceOrigin classify(const std::string& source);

std::string makeSourceName(SourceOrigin origin, const std::string& name);

// Returns true iif Anemomind has the responsibility of the source 
bool sourceIsInternal(const std::string& source);

// Return true iif Anemomind does not have the responsibility of the source
bool sourceIsExternal(const std::string& source);

}  // namespace sail

#endif // ANEMOBOX_SOURCES_H
