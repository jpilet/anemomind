/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TimeStamp.h"
#include <assert.h>
#include <limits>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef HAVE_CLOCK_GETTIME
#include <time.h>
#endif

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
  auto t = TimeStamp(year_ad, month_1to12, day_1to31, hour, minute, seconds);
  if (t.undefined()) {
    LOG(FATAL) << "Failed to create timestamp from year=" << year_ad << " month="
        << month_1to12 << " day=" << day_1to31 << " hour=" << hour << " minute="
        << minute << " seconds=" << seconds;
  }
  return t;
}

TimeStamp TimeStamp::tryUTC(int year_ad, unsigned int month_1to12, unsigned int day_1to31,
          unsigned int hour, unsigned int minute, double seconds) {
  return TimeStamp(year_ad, month_1to12, day_1to31, hour, minute, seconds);
}

TimeStamp TimeStamp::date(int year_ad, unsigned int month_1to12, unsigned int day_1to31) {
  return TimeStamp::UTC(year_ad, month_1to12, day_1to31, 0, 0, 0);
}

TimeStamp TimeStamp::fromTM(const struct tm &tm) {
  return TimeStamp::UTC(1900 + tm.tm_year, tm.tm_mon+1, tm.tm_mday,
                        tm.tm_hour, tm.tm_min, tm.tm_sec);
}

struct tm TimeStamp::makeGMTimeStruct() const {
  time_t rawtime = time_t(_time/TimeRes);
  struct tm result;
  gmtime_r(&rawtime, &result);
  return result;
}

TimeStamp::TimeStamp(int year_ad, int month_1to12, int day_1to31,
    int hour, int minute, double seconds) : _time(UndefinedTime) {
  if (!(inRange(month_1to12, 1, 12) && inRange(day_1to31, 1, 31)
      && inRange(hour, 0, 23) && inRange(minute, 0, 59) && (seconds >= 0))) {
    return;
  }

  struct tm time;
  memset(&time, 0, sizeof(tm));
  time.tm_year = year_ad - 1900;
  time.tm_mon = month_1to12 - 1;
  time.tm_mday = day_1to31;
  time.tm_hour = hour;
  time.tm_min = minute;
  time.tm_sec = int(seconds);

  time_t t = timegm(&time);
  _time = int64_t(t) * 1000 + int64_t((seconds - time.tm_sec) * 1000);
}

TimeStamp TimeStamp::now() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  int64_t t = int64_t(tv.tv_sec) * 1000 + int64_t(tv.tv_usec / 1000);
  return TimeStamp(t);
}

std::string removeFractionalParts(std::string s) {
  int from = s.find('.');
  if (from < s.npos) {
    int to = from+1;
    while (to < s.npos && isdigit(s[to])) {
      to++;
    }
    return removeFractionalParts(s.substr(0, from) + s.substr(to, s.npos - to));
  } else {
    return s;
  }
}

TimeStamp tryParseTime(const char *fmt, std::string s) {
  struct tm tm = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr};

  // http://man7.org/linux/man-pages/man3/strptime.3.html
  auto ret = strptime(s.c_str(), fmt, &tm);

  if (ret == nullptr) {
    return TimeStamp();
  } else if (*ret == 0) {
    return TimeStamp::fromTM(tm);
  }
  return TimeStamp();
}

#define TRY_PARSE_TIME(FMT, X) {auto res = tryParseTime(FMT, X); if (res.defined()) {return res;}}

TimeStamp TimeStamp::parse(const std::string &x0) {

  // TODO: Rewrite this parsing, so that we can also
  // parse fractions of seconds.
  auto x = removeFractionalParts(x0);

  TRY_PARSE_TIME("%D", x);
  TRY_PARSE_TIME("%FT%TZ", x);
  TRY_PARSE_TIME("%F", x);
  TRY_PARSE_TIME("%D %T", x);
  TRY_PARSE_TIME("%m/%d/%Y %r", x);
  LOG(WARNING) << "Failed to parse time: " << x0;
  return TimeStamp();
}

Optional<struct tm> parseTimeToStruct(
    const char* fmt, const std::string& src0) {
  auto src = removeFractionalParts(src0);
  struct tm dst = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr};

  // http://man7.org/linux/man-pages/man3/strptime.3.html
  auto ret = strptime(src.c_str(), fmt, &dst);
  return ret == nullptr? Optional<struct tm>() : dst;
}


TimeStamp TimeStamp::parse(const char* fmt, const std::string &x) {
  for (auto tm: parseTimeToStruct(fmt, x)) {
    return TimeStamp::fromTM(tm);
  }
  return TimeStamp();
}

TimeStamp TimeStamp::makeUndefined() {
  return TimeStamp(UndefinedTime);
}


bool TimeStamp::operator<(const TimeStamp &x) const {
  assert(x.defined());
  CHECK(defined());
  CHECK(x.defined());
  return _time < x._time;
}

double TimeStamp::difSeconds(const TimeStamp &a, const TimeStamp &b) {
  CHECK(a.defined());
  CHECK(b.defined());
  return (1.0/TimeRes)*double(a._time - b._time);
}

std::string TimeStamp::toString(const char *fmt) const {
  assert(defined());
  struct tm time = makeGMTimeStruct();
  const int len = 255;
  char str[len];
  assert(time.tm_gmtoff == 0);
  strftime(str, len, fmt, &time);
  return std::string(str);
}

std::string TimeStamp::toString() const {
  const char isofmt[] = "%FT%T";
  if (defined()) {
    return toString(isofmt);
  }
  return "TimeStamp (undefined)";
}

std::string TimeStamp::fullPrecisionString() const {
  return toString() + stringFormat(".%03d", _time % TimeRes);
}

TimeStamp TimeStamp::offset1970() {
  return TimeStamp::fromMilliSecondsSince1970(0);
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

TimeStamp latest(const TimeStamp &a, const TimeStamp &b) {
  if (a.undefined()) {
    return b;
  } else if (b.undefined()) {
    return a;
  }
  return std::max(a, b);
}

TimeStamp earliest(const TimeStamp &a, const TimeStamp &b) {
  if (a.undefined()) {
    return b;
  } else if (b.undefined()) {
    return a;
  }
  return std::min(a, b);
}


void sleep(Duration<double> duration) {
  usleep(useconds_t(duration.seconds() * 1e6));
}

bool isFinite(const TimeStamp &x) {
  return x.defined();
}

TimeStamp minDefined(TimeStamp a, TimeStamp b) {
  if (a.defined()) {
    if (b.defined()) {
      return std::min(a, b);
    } else {
      return a;
    }
  } else {
    return b;
  }
}

TimeStamp average(const std::initializer_list<TimeStamp>& ts) {
  if (ts.begin() == ts.end()) {
    return TimeStamp();
  }
  TimeStamp offset = *(ts.begin());
  Duration<double> durSum = 0.0_s;
  int n = 0;
  for (auto x: ts) {
    durSum += (x - offset);
    n++;
  }
  return offset + (1.0/n)*durSum;
}


TimeStamp maxDefined(TimeStamp a, TimeStamp b) {
  if (a.defined()) {
    if (b.defined()) {
      return std::max(a, b);
    } else {
      return a;
    }
  } else {
    return b;
  }
}

Optional<Duration<double>> parseTimeOfDay(
    const char* fmt, const std::string& src) {
  for (auto p: parseTimeToStruct(fmt, src)) {
    return double(p.tm_hour)*1.0_h
        + double(p.tm_min)*1.0_minutes
        + double(p.tm_sec)*1.0_s;
  }
  return {};
}


TimeStamp MonotonicClock::now() {
#ifdef HAVE_CLOCK_GETTIME
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return TimeStamp::fromMilliSecondsSince1970(
      int64_t(t.tv_sec) * 1000 + int64_t(t.tv_nsec) / 1000000);
#else
  LOG(FATAL) << "MonotonicClock: clock_gettime is not available.";
  return TimeStamp();
#endif
}

} /* namespace sail */
