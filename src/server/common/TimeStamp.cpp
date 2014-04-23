/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TimeStamp.h"
#include <assert.h>
#include <limits>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/common/mkgmtime.h>

namespace sail {


namespace {
  // Special value reserved for signalling undefined time.
  const int64_t UndefinedTime = std::numeric_limits<int64_t>::max();
  const int TimeRes = 1000; // How precisely we store the time.
}

TimeStamp::TimeStamp(int64_t is) : _time(is) {
}


namespace {
  bool inRange(int x, int a, int b) {
    return a <= x && x <= b;
  }
}

TimeStamp::TimeStamp() : _time(UndefinedTime) {
}

bool TimeStamp::defined() const {
  return _time != UndefinedTime;
}


TimeStamp::TimeStamp(int year_ad, unsigned int month_1to12, unsigned int day_1to31,
          unsigned int hour, unsigned int minute, double seconds, int isdst) {
  assert(inRange(month_1to12, 1, 12));
  assert(inRange(day_1to31, 1, 31));
  assert(inRange(hour, 0, 23));
  assert(inRange(minute, 0, 59));
  assert(seconds >= 0);

  struct tm time;
  time.tm_gmtoff = 0; //gmtoff; //0; offset. http://stackoverflow.com/a/530557
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

struct tm TimeStamp::makeGMTimeStruct() const {
  time_t rawtime = time_t(_time/TimeRes);
  struct tm result;
  gmtime_r(&rawtime, &result);
  return result;
}

void TimeStamp::init(struct tm &time, double fracSeconds) {
  time_t x = mkgmtime(&time);

  assert(x != -1);
  _time = TimeRes*x + int64_t(TimeRes*fracSeconds);
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

TimeStamp TimeStamp::makeUndefined() {
  return TimeStamp(UndefinedTime);
}


bool TimeStamp::operator<(const TimeStamp &x) const {
  CHECK(defined());
  CHECK(x.defined());
  return _time < x._time;
}

double TimeStamp::difSeconds(const TimeStamp &a, const TimeStamp &b) {
  CHECK(a.defined());
  CHECK(b.defined());
  return (1.0/TimeRes)*double(a._time - b._time);
}

Duration<double> operator-(const TimeStamp &a, const TimeStamp &b) {
  return Duration<double>::seconds(TimeStamp::difSeconds(a, b));
}

TimeStamp operator-(const TimeStamp &a, const Duration<double> &b) {
  return a + (-b);
}

TimeStamp operator+(const TimeStamp &a, const Duration<double> &b) {
  CHECK(a.defined());
  CHECK(!std::isnan(b.seconds()));
  int64_t res = a._time + int64_t(TimeRes*b.seconds());
  CHECK(res != UndefinedTime);
  return TimeStamp(res);
}

TimeStamp operator+(const Duration<double> &a, const TimeStamp &b) {
  return b + a;
}

std::ostream &operator<<(std::ostream &s, const TimeStamp &t) {
  struct tm time = t.makeGMTimeStruct();
  const char isofmt[] = "%FT%T";
  const int len = 255;
  char str[len];
  assert(time.tm_gmtoff == 0);
  strftime(str, len, isofmt, &time);
  s << str;
  return s;
}

} /* namespace sail */
