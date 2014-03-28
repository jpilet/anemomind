/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TimeStamp.h"
#include <assert.h>
#include <limits>

namespace sail {

TimeStamp::TimeStamp(IntType is, FracType fs) : _intSeconds(is), _fracSeconds(fs) {
  assert(0 <= fs);
  assert(fs < 1);
}

namespace {
  bool inRange(int x, int a, int b) {
    return a <= x && x <= b;
  }
}

TimeStamp::TimeStamp() : _intSeconds(0), _fracSeconds(std::numeric_limits<double>::signaling_NaN()) {
}

TimeStamp::TimeStamp(int year_ad, unsigned int month_1to12, unsigned int day_1to31,
          unsigned int hour, unsigned int minute, double seconds, int gmtoff, int isdst) {
  assert(inRange(month_1to12, 1, 12));
  assert(inRange(day_1to31, 1, 31));
  assert(inRange(hour, 0, 23));
  assert(inRange(minute, 0, 59));
  assert(seconds >= 0);

  struct tm time;
  time.tm_gmtoff = gmtoff; //0;
  time.tm_isdst = isdst; //0; // daylight saving. What to put here???
  time.tm_sec = int(seconds);
  time.tm_min = minute;
  time.tm_hour = hour;
  time.tm_mon = month_1to12 - 1;
  time.tm_year = year_ad - 1900;
  time.tm_mday = day_1to31;

  // not used
  time.tm_yday = -1;
  time.tm_wday = -1;


  init(time, seconds - time.tm_sec);
}

void TimeStamp::init(struct tm &time, double fracSeconds) {
  time_t x = mktime(&time);

  assert(x != -1);

  _intSeconds = x;
  _fracSeconds = fracSeconds;
}



bool TimeStamp::operator<(const TimeStamp &x) const {
  assert(defined());
  assert(x.defined());
  if (_intSeconds < x._intSeconds) {
    return true;
  } else if (_intSeconds > x._intSeconds) {
    return false;
  } else {
    return _fracSeconds < x._fracSeconds;
  }
}

double TimeStamp::difSeconds(const TimeStamp &a, const TimeStamp &b) {
  return double(a._intSeconds - b._intSeconds) + (a._fracSeconds - b._fracSeconds);
}

Duration<double> operator-(const TimeStamp &a, const TimeStamp &b) {
  return Duration<double>::seconds(TimeStamp::difSeconds(a, b));
}

TimeStamp operator-(const TimeStamp &a, const Duration<double> &b) {
  return a + (-b);
}

TimeStamp operator+(const TimeStamp &a, const Duration<double> &b) {
  TimeStamp::IntType bi = TimeStamp::IntType(b.seconds());
  TimeStamp::IntType ci = a._intSeconds + bi;
  TimeStamp::FracType cf = a._fracSeconds + (b.seconds() - bi);
  if (cf >= 1) {
    ci++;
    cf -= 1.0;
  } else if (cf <= -1) {
    ci--;
    cf += 1.0;
  }
  return TimeStamp(ci, cf);
}

TimeStamp operator+(const Duration<double> &a, const TimeStamp &b) {
  return b + a;
}

} /* namespace sail */
