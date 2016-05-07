#include <device/anemobox/Dispatcher.h>

#include <assert.h>

namespace sail {

Dispatcher *Dispatcher::_globalInstance = 0;

const char* descriptionForCode(DataCode code) {
  switch (code) {
#define CASE_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    case HANDLE : return DESCRIPTION;

  FOREACH_CHANNEL(CASE_ENTRY)
#undef CASE_ENTRY
  }
}

const char* wordIdentifierForCode(DataCode code) {
  switch (code) {
#define CASE_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    case HANDLE : return SHORTNAME;

  FOREACH_CHANNEL(CASE_ENTRY)
#undef CASE_ENTRY
  }
}

Dispatcher::Dispatcher() {
  // Instanciates a proxy for each channel.
#define REGISTER_PROXY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  _currentSource[HANDLE] = std::shared_ptr<DispatchDataProxy<TYPE>>( \
      new DispatchDataProxy<TYPE>(HANDLE));
  FOREACH_CHANNEL(REGISTER_PROXY);
#undef REGISTER_PROXY
}

Dispatcher *Dispatcher::global() {
  if (!_globalInstance) {
    _globalInstance = new Dispatcher();
  }
  return _globalInstance;
}

  std::shared_ptr<DispatchData> Dispatcher::dispatchDataForSource(DataCode code, const std::string& source) {
    auto sourcesForCode = _data[code];
    auto sourceIt = sourcesForCode.find(source);
    if (sourceIt == sourcesForCode.end()) {
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
    return 0;
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

}  // namespace sail
