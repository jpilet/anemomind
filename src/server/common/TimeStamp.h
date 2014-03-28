/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <ctime>

#include "PhysicalQuantity.h"

namespace sail {

// A more accurate and safe type that time_t in <ctime>
class TimeStamp {
 public:
  typedef long long int IntType;

  TimeStamp(int year_ad, unsigned int month_1to12, unsigned int day_1to31,
            unsigned int hour, unsigned int minute, double seconds,
            int gmtoff=0, int isdst=0);

  TimeStamp(struct tm time);


  static TimeStamp now();


  bool operator<(const TimeStamp &x) const;
  bool operator>(const TimeStamp &x) const {return x < (*this);}
  bool operator<=(const TimeStamp &x) const {return !(x < (*this));}
  bool operator>=(const TimeStamp &x) const {return !((*this) < x);}

  TimeStamp(); // Default contructor of an object with defined() returning false.
  bool defined() const;
 private:
  void init(struct tm &time, double fracSeconds);
  TimeStamp(IntType is);
  static double difSeconds(const TimeStamp &a, const TimeStamp &b);

  friend Duration<double> operator-(const TimeStamp &a, const TimeStamp &b);
  friend TimeStamp operator+(const TimeStamp &a, const Duration<double> &b);

  // _intSeconds + _fracSeconds is the total number of seconds since 1970

  // Integral part of seconds since 1970
  IntType _milliSeconds;
};

Duration<double> operator-(const TimeStamp &a, const TimeStamp &b);
TimeStamp operator-(const TimeStamp &a, const Duration<double> &b);
TimeStamp operator+(const TimeStamp &a, const Duration<double> &b);
TimeStamp operator+(const Duration<double> &a, const TimeStamp &b);

} /* namespace sail */

#endif /* TIMESTAMP_H_ */
