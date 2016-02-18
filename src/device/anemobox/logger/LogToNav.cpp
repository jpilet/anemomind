
#include <device/anemobox/logger/LogToNav.h>

#include <device/anemobox/DispatcherTrueWindEstimator.h>
#include <device/anemobox/logger/Logger.h>
#include <server/common/TimeStamp.h>
#include <server/common/logging.h>
#include <server/common/ArrayBuilder.h>
#include <set>
#include <vector>

using namespace std;

namespace sail {

namespace {

bool all(const string&) { return true; }

const ValueSet* searchFor(string shortName, const LogFile& data,
                          bool (sourceRestrict)(const string&) = all) {
  const ValueSet* result = 0;
  int priority = 0;

  for (int i = 0; i < data.stream_size(); ++i) {
    const ValueSet& stream = data.stream(i);
    if (stream.shortname() == shortName && sourceRestrict(stream.source())) {
      int newPriority = stream.has_priority() ? stream.priority() : 0;
      if (result == 0 || newPriority > priority) {
        priority = newPriority;
        result = &stream;
      }
    }
  }
  return result;
}

bool internalSources(const string& source) {
  return source == DispatcherTrueWindEstimator::sourceName();
}

bool externalSources(const string& source) {
  return !internalSources(source);
}

Array<Nav> makePosArray(const LogFile& data) {
  const ValueSet* pos = searchFor("pos", data);
  if (pos == 0 || !pos->has_pos()) {
    LOG(ERROR) << "Log file has no 'pos' data";
    return Array<Nav>();
  }

  vector<TimeStamp> timestamps;
  Logger::unpackTime(*pos, &timestamps);

  vector<GeographicPosition<double>> values;
  Logger::unpack(pos->pos(), &values);

  if (timestamps.size() != values.size()) {
    LOG(ERROR) << "malformed log file: " << timestamps.size() << " times, "
      << values.size() << " pos";
    return Array<Nav>();
  }

  ArrayBuilder<Nav> result;

  auto minDelta = Duration<>::seconds(.09);
  for (int i = 0; i < timestamps.size(); ++i) {
    if (result.size() == 0 || (timestamps[i] - result.last().time()).fabs() > minDelta) {
      Nav nav;
      nav.setTime(timestamps[i]);
      nav.setGeographicPosition(values[i]);
      result.add(nav);
    }
  }
  return result.get();
}

int nearest(const TimeStamp& key, const vector<TimeStamp>& timestamps) {
  if (timestamps.size() == 0) {
    return -1;
  }

  int low = 0;
  int high = timestamps.size() - 1;

  if (key < timestamps[low]) {
    return low;
  }
  if (key > timestamps[high]) {
    return high;
  }

  while ((high - low) > 1) {
    int mid = (low + high) / 2;
    if (key >= timestamps[mid]) {
      low = mid;
    } else {
      high = mid;
    }
  }

  auto lowDiff = key - timestamps[low];
  auto highDiff = timestamps[high] - key;
  return (lowDiff < highDiff ? low : high);
}

template<class T, class ValueSetType>
void fill(const ValueSet* valueSet, const ValueSetType& packed, 
          Array<Nav>* array, void (Nav::* set)(T)) {
  vector<TimeStamp> timestamps;
  Logger::unpackTime(*valueSet, &timestamps);
  vector<T> unpacked;
  Logger::unpack(packed, &unpacked);
  for (int i = 0; i < array->size(); ++i) {
    Nav* nav = &((*array)[i]);
    int j = nearest(nav->time(), timestamps);
    if (j >= 0) {
      (nav->*set)(unpacked[j]);
    }
  }
}

}  // namespace

NavCollection logFileToNavArray(const LogFile& data) {
  Array<Nav> result = makePosArray(data);

  const ValueSet* awa = searchFor("awa", data);
  if (awa != 0 && awa->has_angles()) {
    fill(awa, awa->angles(), &result, &Nav::setAwa);
  }

  const ValueSet* aws = searchFor("aws", data);
  if (aws != 0 && aws->has_velocity()) {
    fill(aws, aws->velocity(), &result, &Nav::setAws);
  }

  const ValueSet* twa = searchFor("twa", data, externalSources);
  if (twa != 0 && twa->has_angles()) {
    fill(twa, twa->angles(), &result, &Nav::setExternalTwa);
  }

  const ValueSet* tws = searchFor("tws", data, externalSources);
  if (tws != 0 && tws->has_velocity()) {
    fill(tws, tws->velocity(), &result, &Nav::setExternalTws);
  }

  const ValueSet* devTwa = searchFor("twa", data, internalSources);
  if (devTwa != 0 && devTwa->has_angles()) {
    fill(devTwa, devTwa->angles(), &result, &Nav::setDeviceTwa);
  }

  const ValueSet* devTws = searchFor("tws", data, internalSources);
  if (devTws != 0 && devTws->has_velocity()) {
    fill(devTws, devTws->velocity(), &result, &Nav::setDeviceTws);
  }

  const ValueSet* twdir = searchFor("twdir", data, internalSources);
  if (twdir != 0 && twdir->has_angles()) {
    fill(twdir, twdir->angles(), &result, &Nav::setDeviceTwdir);
  }

  const ValueSet* gpsBearing = searchFor("gpsBearing", data);
  if (gpsBearing  != 0 && gpsBearing->has_angles()) {
    fill(gpsBearing, gpsBearing->angles(), &result, &Nav::setGpsBearing);
  }

  const ValueSet* valueSet = searchFor("gpsSpeed", data);
  if (valueSet != 0 && valueSet->has_velocity()) {
    fill(valueSet, valueSet->velocity(), &result, &Nav::setGpsSpeed);
  }

  valueSet = searchFor("magHdg", data);
  if (valueSet != 0 && valueSet->has_angles()) {
    fill(valueSet, valueSet->angles(), &result, &Nav::setMagHdg);
  }

  valueSet = searchFor("watSpeed", data);
  if (valueSet != 0 && valueSet->has_velocity()) {
    fill(valueSet, valueSet->velocity(), &result, &Nav::setWatSpeed);
  }

  valueSet = searchFor("targetVmg", data);
  if (valueSet != 0 && valueSet->has_velocity()) {
    fill(valueSet, valueSet->velocity(), &result, &Nav::setDeviceTargetVmg);
  }

  valueSet = searchFor("vmg", data);
  if (valueSet != 0 && valueSet->has_velocity()) {
    fill(valueSet, valueSet->velocity(), &result, &Nav::setDeviceVmg);
  }

  return NavCollection::fromNavs(result);
}

NavCollection logFileToNavArray(const std::string& filename) {
  LogFile data;
  if (Logger::read(filename, &data)) {
    return logFileToNavArray(data);
  }
  return NavCollection();
}

} // namespace sail
