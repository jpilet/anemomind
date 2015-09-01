#include <device/anemobox/Dispatcher.h>

#include <assert.h>

namespace sail {

Dispatcher *Dispatcher::_globalInstance = 0;

const char* descriptionForCode(DataCode code) {
  assert(code > 0 && code < NUM_DATA_CODE);
  switch (code) {
    case AWA: return "apparent wind angle";
    case AWS: return "apparent wind speed";
    case TWA: return "true wind angle";
    case TWS: return "true wind speed";
    case TWDIR: return "true wind direction";
    case GPS_BEARING: return "GPS bearing";
    case GPS_SPEED: return "GPS speed";
    case MAG_HEADING: return "magnetic heading";
    case WAT_SPEED: return "water speed";
    case WAT_DIST: return "distance over water";
    case GPS_POS: return "GPS position";
    case DATE_TIME: return "GPS date and time (UTC)";
    case TARGET_VMG: return "Target VMG";
    case VMG: return "VMG";
    case ORIENT: return "absolute orientation";
    case NUM_DATA_CODE: return "INVALID CODE";
    // No default: the compiler will tell us if an entry is missing.
  }
}

const char* wordIdentifierForCode(DataCode code) {
  assert(code > 0 && code < NUM_DATA_CODE);
  switch (code) {
    case AWA: return "awa";
    case AWS: return "aws";
    case TWA: return "twa";
    case TWS: return "tws";
    case TWDIR: return "twdir";
    case GPS_BEARING: return "gpsBearing";
    case GPS_SPEED: return "gpsSpeed";
    case MAG_HEADING: return "magHdg";
    case WAT_SPEED: return "watSpeed";
    case WAT_DIST: return "watDist";
    case GPS_POS: return "pos";
    case DATE_TIME: return "dateTime";
    case TARGET_VMG: return "targetVmg";
    case VMG: return "vmg";
    case ORIENT: return "orient";
    case NUM_DATA_CODE: return "INVALID CODE";
    // No default: the compiler will tell us if an entry is missing.
  }
}


template <DataCode Code> void Dispatcher::registerCode() {
  typedef typename TypeForCode<Code>::type CodeType;
  _currentSource[Code] = std::shared_ptr<DispatchDataProxy<CodeType>>(
      new DispatchDataProxy<CodeType>(Code));

  registerCode<static_cast<DataCode>(Code + 1)>();
}
template <> void Dispatcher::registerCode<NUM_DATA_CODE>() { }

Dispatcher::Dispatcher() {
  // Instanciates a proxy for each code.
  registerCode<AWA>();
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
