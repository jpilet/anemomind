/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <ctime>
#include <cinttypes>

#include "PhysicalQuantity.h"

namespace sail {

// A more accurate and safe type than time_t in <ctime>
class TimeStamp {
 public:

  TimeStamp(int year_ad, unsigned int month_1to12, unsigned int day_1to31,
            unsigned int hour, unsigned int minute, double seconds,
            int gmtoff=0, int isdst=0);

  TimeStamp(struct tm time);

  TimeStamp(const TimeStamp &) = default;

  static TimeStamp now();
  static TimeStamp makeUndefined();


  bool operator<(const TimeStamp &x) const;
  bool operator>(const TimeStamp &x) const {return x < (*this);}
  bool operator<=(const TimeStamp &x) const {return !(x < (*this));}
  bool operator>=(const TimeStamp &x) const {return !((*this) < x);}

  TimeStamp(); // Default contructor of an object with defined() returning false.

  bool defined() const;
  bool undefined() const {return !defined();}

  // Used by the Json interface
  static TimeStamp fromInteger(int64_t x) {return TimeStamp(x);}
  int64_t toInteger() const {return _time;}
 private:
  void init(struct tm &time, double fracSeconds);
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

} /* namespace sail */

#endif /* TIMESTAMP_H_ */
