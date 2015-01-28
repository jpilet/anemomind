/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <Poco/DateTime.h>
#include "TimeStamp.h"
#include <assert.h>
#include <limits>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <Poco/Timestamp.h>

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


TimeStamp TimeStamp::UTC(int year_ad, unsigned int month_1to12, unsigned int day_1to31,
          unsigned int hour, unsigned int minute, double seconds) {
  assert(inRange(month_1to12, 1, 12));
  assert(inRange(day_1to31, 1, 31));
  assert(inRange(hour, 0, 23));
  assert(inRange(minute, 0, 59));
  assert(seconds >= 0);

  unsigned int intSecs = int(seconds);
  double fracSecs = seconds - intSecs;

  return TimeStamp(year_ad, month_1to12, day_1to31,
                   hour, minute, intSecs, fracSecs);
}

TimeStamp TimeStamp::date(int year_ad, unsigned int month_1to12, unsigned int day_1to31) {
  return TimeStamp::UTC(year_ad, month_1to12, day_1to31, 0, 0, 0);
}

struct tm TimeStamp::makeGMTimeStruct() const {
  time_t rawtime = time_t(_time/TimeRes);
  struct tm result;
  gmtime_r(&rawtime, &result);
  return result;
}

TimeStamp::TimeStamp(int year, int mon, int day,
    int hour, int min, int sec, double fracSeconds) {
  double fmillis = fracSeconds*1000;
  int millis = int(fmillis);
  int micros = int(1000*(fmillis - millis));
  assert(inRange(millis, 0, 999));
  assert(inRange(micros, 0, 999));
  Poco::DateTime dt(year, mon, day,
                    hour, min, sec, millis, micros);
  init(dt);
}

void TimeStamp::init(const Poco::DateTime &dt) {
  Poco::Timestamp::UtcTimeVal utcval = dt.utcTime();
  Poco::Timestamp ts = Poco::Timestamp::fromUtcTime(utcval);
  time_t x = ts.epochTime();

  double frac = 1.0e-6*(ts.epochMicroseconds() - 1.0e6*x);

  assert(x != -1);
  _time = TimeRes*x + int64_t(TimeRes*frac);
}


TimeStamp TimeStamp::now() {
  Poco::DateTime dt;
  return TimeStamp(dt);
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

std::string TimeStamp::toString() const {
  struct tm time = makeGMTimeStruct();
  const char isofmt[] = "%FT%T";
  const int len = 255;
  char str[len];
  assert(time.tm_gmtoff == 0);
  strftime(str, len, isofmt, &time);
  return std::string(str);
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
  return s << t.toString();
}

} /* namespace sail */
