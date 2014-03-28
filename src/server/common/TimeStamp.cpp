/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TimeStamp.h"
#include <assert.h>
#include <limits>

namespace sail {


namespace {
  // Special value reserved for signalling undefined time.
  const TimeStamp::IntType UndefinedTime = std::numeric_limits<TimeStamp::IntType>::max();
}

TimeStamp::TimeStamp(IntType is) : _milliSeconds(is) {
  assert(is != UndefinedTime);
}


namespace {
  bool inRange(int x, int a, int b) {
    return a <= x && x <= b;
  }
}

TimeStamp::TimeStamp() : _milliSeconds(UndefinedTime) {
}

bool TimeStamp::defined() const {
  return _milliSeconds != UndefinedTime;
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
  _milliSeconds = 1000*x + IntType(1000*fracSeconds);
}

TimeStamp::TimeStamp(struct tm time) {
  init(time, 0.0);
}

TimeStamp TimeStamp::now() {
  time_t rawtime;
  time(&rawtime);
  struct tm *timeinfo = localtime(&rawtime);
  return TimeStamp(*timeinfo);
}


bool TimeStamp::operator<(const TimeStamp &x) const {
  assert(defined());
  assert(x.defined());
  return _milliSeconds < x._milliSeconds;
}

double TimeStamp::difSeconds(const TimeStamp &a, const TimeStamp &b) {
  return 0.001*double(a._milliSeconds - b._milliSeconds);
}

Duration<double> operator-(const TimeStamp &a, const TimeStamp &b) {
  return Duration<double>::seconds(TimeStamp::difSeconds(a, b));
}

TimeStamp operator-(const TimeStamp &a, const Duration<double> &b) {
  return a + (-b);
}

TimeStamp operator+(const TimeStamp &a, const Duration<double> &b) {
  return TimeStamp(a._milliSeconds + TimeStamp::IntType(1000.0*b.seconds()));
}

TimeStamp operator+(const Duration<double> &a, const TimeStamp &b) {
  return b + a;
}

} /* namespace sail */
