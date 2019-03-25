
#include <string>

namespace sail {

class LogAccumulator;

bool parseIwatch(const std::string& filename, LogAccumulator* dst);

}  // namespace sail
