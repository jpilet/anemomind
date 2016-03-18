/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 *  Helper code for loading data.
 */

#ifndef SERVER_NAUTICAL_LOGIMPORT_SOURCEGROUP_H_
#define SERVER_NAUTICAL_LOGIMPORT_SOURCEGROUP_H_

#include <server/common/TimeStamp.h>
#include <device/anemobox/TimedSampleCollection.h>
#include <server/nautical/GeographicPosition.h>
#include <map>

namespace sail {

class LogLoader;

template <typename T>
typename TimedSampleCollection<T>::TimedVector
  *allocateSourceIfNeeded(const std::string &name, std::map<std::string, typename TimedSampleCollection<T>::TimedVector> *sources) {
  assert(sources != nullptr);
  auto found = sources->find(name);
  if (found == sources->end()) {
    auto &dst = (*sources)[name];
    return &dst;
  }
  return &(found->second);
}

template <typename T>
void pushBack(TimeStamp time, const T &x, typename TimedSampleCollection<T>::TimedVector *dst) {
  if (time.defined() && sail::isFinite(x)) {
    dst->push_back(TimedValue<T>(time, x));
  }
}

struct SourceGroup {
  SourceGroup();
  SourceGroup(const std::string &srcName, LogLoader *dst);

  TimeStamp lastTime;

  TimedSampleCollection<Angle<double> >::TimedVector *awa;
  TimedSampleCollection<Velocity<double> >::TimedVector *aws;
  TimedSampleCollection<Angle<double> >::TimedVector *twa;
  TimedSampleCollection<Velocity<double> >::TimedVector *tws;
  TimedSampleCollection<Angle<double> >::TimedVector *magHdg;
  TimedSampleCollection<Velocity<double> >::TimedVector *watSpeed;
  TimedSampleCollection<GeographicPosition<double> >::TimedVector *geoPos;
  TimedSampleCollection<Angle<double> >::TimedVector *gpsBearing;
  TimedSampleCollection<Velocity<double> >::TimedVector *gpsSpeed;
};

}

#endif /* SERVER_NAUTICAL_LOGIMPORT_SOURCEGROUP_H_ */
