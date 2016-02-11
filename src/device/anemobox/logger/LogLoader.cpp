/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/logger/LogLoader.h>
#include <device/anemobox/logger/Logger.h>

namespace sail {

template <typename T>
void addToVector(const ValueSet &src, std::deque<TimedValue<T> > *dst) {
  std::vector<TimeStamp> timeVector;
  std::vector<T> dataVector;
  ValueSetToTypedVector<TimeStamp>::extract(src, &timeVector);
  ValueSetToTypedVector<T>::extract(src, &dataVector);
  auto n = dataVector.size();
  if (n == dataVector.size()) {
    for (size_t i = 0; i < n; i++) {
      dst->push_back(TimedValue<T>(timeVector[i], dataVector[i]));
    }
  } else {
    LOG(WARNING) << "Incompatible time and data vector sizes. Ignore this data.";
  }
}

void LogLoader::load(const LogFile &data) {
  for (int i = 0; i < data.stream_size(); i++) {
    const auto &stream = data.stream(i);
    _sourcePriority[stream.source()] = stream.priority();

#define ADD_VALUES_TO_VECTOR(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  if (stream.shortname() == SHORTNAME) {addToVector<TYPE>(stream, &(_##HANDLE##sources[stream.source()]));}
      FOREACH_CHANNEL(ADD_VALUES_TO_VECTOR)
#undef  ADD_VALUES_TO_VECTOR

  }
}

void LogLoader::load(const std::string &filename) {
  LogFile file;
  if (Logger::read(filename, &file)) {
    load(file);
  }
}

template <typename T>
void insertValues(DataCode code,
    const std::map<std::string, typename TimedSampleCollection<T>::TimedVector> &src,
    Dispatcher *dst) {
  for (auto kv: src) {
    dst->insertValues<T>(code, kv.first, kv.second);
  }
}

void LogLoader::addToDispatcher(std::shared_ptr<Dispatcher> dst) {

#define INSERT_VALUES(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
    insertValues<TYPE>(HANDLE, _##HANDLE##sources, dst.get());
    FOREACH_CHANNEL(INSERT_VALUES);
#undef  INSERT_VALUES

  for (auto kv: _sourcePriority) {
    dst->setSourcePriority(kv.first, kv.second);
  }
}

}
