/*
 * Channels.h
 *
 *  Created on: 3 Aug 2017
 *      Author: jonas
 */

#ifndef DEVICE_ANEMOBOX_CHANNELS_H_
#define DEVICE_ANEMOBOX_CHANNELS_H_

namespace sail {
/*
 List of channels used by Dispatcher and NavHistory.

 This macro provides an easy way to iterate over channels at compile time.

 To use this macro, define a macro that takes the following arguments:
  #define ENUM_ENTRY(handle, code, shortname, type, description)

 For example, here's how to declare a switch for each entry:

 #define CASE_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
     case handle : return shortname;

   FOREACH_CHANNEL(CASE_ENTRY)
 #undef CASE_ENTRY

 Rules to follow for modifying this list:
 - never remove any entry
 - never change an existing shortname
 - never change an existing code
 otherwise this might break compatibility with recorded data.
*/
#define FOREACH_CHANNEL(X) \
  X(AWA, 1, "awa", Angle<>, "apparent wind angle") \
  X(AWS, 2, "aws", Velocity<>, "apparent wind speed") \
  X(TWA, 3, "twa", Angle<>, "true wind angle") \
  X(TWS, 4, "tws", Velocity<>, "true wind speed") \
  X(TWDIR, 5, "twdir", Angle<>, "true wind direction") \
  X(GPS_SPEED, 6, "gpsSpeed", Velocity<>, "GPS speed") \
  X(GPS_BEARING, 7, "gpsBearing", Angle<>, "GPS bearing") \
  X(MAG_HEADING, 8, "magHdg", Angle<>, "magnetic heading") \
  X(WAT_SPEED, 9, "watSpeed", Velocity<>, "water speed") \
  X(WAT_DIST, 10, "watDist", Length<>, "distance over water") \
  X(GPS_POS, 11, "pos", GeographicPosition<double>, "GPS position") \
  X(DATE_TIME, 12, "dateTime", TimeStamp, "GPS date and time (UTC)") \
  X(TARGET_VMG, 13, "targetVmg", Velocity<>, "Target VMG") \
  X(VMG, 14, "vmg", Velocity<>, "VMG") \
  X(ORIENT, 15, "orient", AbsoluteOrientation, "Absolute anemobox orientation") \
  X(RUDDER_ANGLE, 16, "rudderAngle", Angle<>, "Rudder angle") \
  X(VALID_GPS, 17, "validGps", BinaryEdge, "Valid GPS periods")

enum DataCode {
#define ENUM_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  HANDLE = CODE,

  FOREACH_CHANNEL(ENUM_ENTRY)
#undef ENUM_ENTRY
};
}

namespace std {
  template <>
  struct hash<sail::DataCode> {
    size_t operator()(sail::DataCode c) const {
      return static_cast<size_t>(c);
    }
  };
}

#endif /* DEVICE_ANEMOBOX_CHANNELS_H_ */
