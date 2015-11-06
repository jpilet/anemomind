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

  DispatchData *Dispatcher::dispatchDataForSource(DataCode code, const std::string& source) {
    auto sourcesForCode = _data[code];
    auto sourceIt = sourcesForCode.find(source);
    if (sourceIt == sourcesForCode.end()) {
      return nullptr;
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

  int Dispatcher::sourcePriority(const std::string& source) {
    auto it = _sourcePriority.find(source);
    if (it != _sourcePriority.end()) {
      return it->second;
    }
    return 0;
  }

  void Dispatcher::setSourcePriority(const std::string& source, int priority) {
    if (priority == 0) {
      _sourcePriority.erase(source);
    } else {
      _sourcePriority[source] = priority;
    }
  }
}  // namespace sail
