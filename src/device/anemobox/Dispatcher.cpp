#include <device/anemobox/Dispatcher.h>
#include <server/common/string.h>

#include <assert.h>

namespace sail {

const char* descriptionForCode(DataCode code) {
  switch (code) {
#define CASE_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    case HANDLE : return DESCRIPTION;

  FOREACH_CHANNEL(CASE_ENTRY)
#undef CASE_ENTRY
  }
  return nullptr;
}

const char* wordIdentifierForCode(DataCode code) {
  switch (code) {
#define CASE_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    case HANDLE : return SHORTNAME;

  FOREACH_CHANNEL(CASE_ENTRY)
#undef CASE_ENTRY
  }
  return nullptr;
}

Dispatcher::Dispatcher() {
  // Instanciates a proxy for each channel.
#define REGISTER_PROXY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  _currentSource[HANDLE] = std::shared_ptr<DispatchDataProxy<TYPE>>( \
      new DispatchDataProxy<TYPE>(HANDLE));
  FOREACH_CHANNEL(REGISTER_PROXY);
#undef REGISTER_PROXY
}

  std::shared_ptr<DispatchData> Dispatcher::dispatchDataForSource(DataCode code, const std::string& source) const {
    auto sourcesForCode = _data.find(code);
    if (sourcesForCode == _data.end()) {
      return std::shared_ptr<DispatchData>();
    }

    auto sourceIt = sourcesForCode->second.find(source);
    if (sourceIt == sourcesForCode->second.end()) {
      return std::shared_ptr<DispatchData>();
    }
    return sourceIt->second;
  }

  bool Dispatcher::prefers(DispatchData* a, DispatchData* b) {
    if (a == b) {
      return false;
    }
    if (b == 0) {
      return true;
    }
    if (!b->isFresh()) {
      return true;
    }
    return sourcePriority(a->source()) > sourcePriority(b->source());
  }

  int getSourcePriority(const std::map<std::string, int> &sourcePriority, const std::string &source) {
    auto it = sourcePriority.find(source);
    if (it != sourcePriority.end()) {
      return it->second;
    }

    if (source.size() > 5) {
      if (startsWith(source, "mix (")) {
        return 100;
      } else if (startsWith(source, "NMEA2000/")) {
        return 5;
      } else if (startsWith(source, "iWatch")) {
        return -1;
      }
    }

    return Dispatcher::defaultPriority;
  }

  int Dispatcher::sourcePriority(const std::string& source) const {
    return getSourcePriority(_sourcePriority, source);
  }

  void Dispatcher::setSourcePriority(const std::string& source, int priority) {
    if (priority == 0) {
      _sourcePriority.erase(source);
    } else {
      _sourcePriority[source] = priority;
    }
  }

  void Dispatcher::set(DataCode code, const std::string &srcName,
      const std::shared_ptr<DispatchData> &d) {
    _data[code][srcName] = d;
  }

  int Dispatcher::maxPriority() const {
    int m = Dispatcher::defaultPriority;
    for (auto kv: _sourcePriority) {
      m = std::max(m, kv.second);
    }
    return m;
  }

const std::vector<DataCode>& allDataCodes() {
  static std::vector<DataCode> codes{
 #define ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
     HANDLE,
   FOREACH_CHANNEL(ENTRY)
 #undef ENTRY
  };

  return codes;
}

}  // namespace sail
