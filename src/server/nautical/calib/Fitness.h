/*
 * Fitness.h
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 *
 * This code is about generating the residuals for fitting to data.
 */

#ifndef SERVER_NAUTICAL_CALIB_FITNESS_H_
#define SERVER_NAUTICAL_CALIB_FITNESS_H_

#include <device/anemobox/Dispatcher.h>

namespace sail {


template <DataCode code>
struct SensorModel {};

struct BasicAngleSensor {};

template <>
struct SensorModel<AWA> : public BasicAngleSensor {};

struct SensorSet {
#define MAKE_SENSOR_FIELD(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, SensorModel<HANDLE> > HANDLE;
FOREACH_CHANNEL(MAKE_SENSOR_FIELD)
#undef MAKE_SENSOR_FIELD

};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
