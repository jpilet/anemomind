/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <ctime>
#include <cinttypes>
#include <iosfwd>

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

// A more accurate and safe type than time_t in <ctime>
class TimeStamp {
 public:

  static TimeStamp UTC(int year_ad, unsigned int month_1to12, unsigned int day_1to31,
            unsigned int hour, unsigned int minute, double seconds);

  static TimeStamp date(int year_ad, unsigned int month_1to12, unsigned int day_1to31);

  TimeStamp(const TimeStamp &) = default;

  static TimeStamp fromTM(const struct tm &tm);

  static TimeStamp now();
  static TimeStamp makeUndefined();

  static TimeStamp parse(const std::string &x);


  bool operator<(const TimeStamp &x) const;
  bool operator>(const TimeStamp &x) const {return x < (*this);}
  bool operator<=(const TimeStamp &x) const {return !(x < (*this));}
  bool operator>=(const TimeStamp &x) const {return !((*this) < x);}

  TimeStamp(); // Default contructor of an object with defined() returning false.

  bool defined() const;
  bool undefined() const {return !defined();}

  std::string toString(const char *fmt) const;
  std::string toString() const;
  std::string fullPrecisionString() const;

  std::string toIso8601String() const { return toString("%FT%TZ"); }

  // Used by the Json interface
  static TimeStamp fromMilliSecondsSince1970(int64_t x) {return TimeStamp(x);}
  static TimeStamp offset1970();

  int64_t toMilliSecondsSince1970() const {return _time;}
  int64_t toSecondsSince1970() const {return toMilliSecondsSince1970()/int64_t(1000);}
  struct tm makeGMTimeStruct() const;

  bool operator== (const TimeStamp &other) const {
    return _time == other._time;
  }
 private:
  TimeStamp(int year, int mon, int day, int hour, int min, double seconds);

  TimeStamp(int64_t is);
  static double difSeconds(const TimeStamp &a, const TimeStamp &b);

  friend Duration<double> operator-(const TimeStamp &a, const TimeStamp &b);
  friend TimeStamp operator+(const TimeStamp &a, const Duration<double> &b);

  int64_t _time;
};

Duration<double> operator-(const TimeStamp &a, const TimeStamp &b);
TimeStamp operator-(const TimeStamp &a, const Duration<double> &b);
TimeStamp operator+(const TimeStamp &a, const Duration<double> &b);
TimeStamp operator+(const Duration<double> &a, const TimeStamp &b);

std::ostream &operator<<(std::ostream &s, const TimeStamp &t);

class Clock {
 public:
  virtual ~Clock() { }
  virtual TimeStamp currentTime() { return TimeStamp::now(); }
};

void sleep(Duration<double> duration);

} /* namespace sail */

#endif /* TIMESTAMP_H_ */
