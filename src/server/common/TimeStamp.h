/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include "PhysicalQuantity.h"

namespace sail {

class TimeStamp {
 public:
  typedef long long int IntType;
  typedef double FracType;

  TimeStamp(int year_ad, unsigned int month_1to12, unsigned int day_1to31,
            unsigned int hour, unsigned int minute, double seconds,
            int gmtoff=0, int isdst=0);

  TimeStamp(struct tm time);
  TimeStamp();

  bool operator<(const TimeStamp &x) const;

  bool defined() const {return !std::isnan(_fracSeconds);}
 private:
  void init(struct tm &time, double fracSeconds);
  TimeStamp(IntType is, FracType fs);
  static double difSeconds(const TimeStamp &a, const TimeStamp &b);

  friend Duration<double> operator-(const TimeStamp &a, const TimeStamp &b);

  // _intSeconds + _fracSeconds is the total number of seconds since

  // Integral part of seconds since 1970
  IntType _intSeconds;

  // Fractional part of seconds since 1970. In range [0, 1[
  FracType _fracSeconds;
};

Duration<double> operator-(const TimeStamp &a, const TimeStamp &b);

} /* namespace sail */

#endif /* TIMESTAMP_H_ */
