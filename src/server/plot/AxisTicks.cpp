/*
 * AxisTicks.cpp
 *
 *  Created on: 22 Oct 2016
 *      Author: jonas
 */

#include <server/plot/AxisTicks.h>
#include <server/common/String.h>
#include <server/common/TimeStamp.h>
#include <iostream>

namespace sail {

BasicTickIterator::BasicTickIterator(int e, const std::string &unit) :
    _exponent(e), _unit(unit) {}

BasicTickIterator BasicTickIterator::coarser() const {
  return BasicTickIterator(_exponent + 1, _unit);
}

BasicTickIterator BasicTickIterator::finer() const {
  return BasicTickIterator(_exponent - 1, _unit);
}

double BasicTickIterator::computeFracIndex(double value) const {
  return value/tickSpacing();
}

AxisTick<double> BasicTickIterator::get(int index) const {
  auto x = index*tickSpacing();
  auto label = stringFormat("%.3g %s", x, _unit.c_str());
  return AxisTick<double>{x, label};
}

double BasicTickIterator::tickSpacing() const {
  return std::pow(10.0, double(_exponent));
}

int timeStampToIndex(TimeStamp t) {
  auto x = t.makeGMTimeStruct();
  return ((x.tm_year + 1900)*12 + x.tm_mon) + (x.tm_mday-1)/30.0;
}

DateTickIterator::DateTickIterator(int l) : _level(l) {}

DateTickIterator DateTickIterator::finer() const {
  return DateTickIterator(std::max(0, _level - 1));
}

DateTickIterator DateTickIterator::coarser() const {
  return DateTickIterator(_level + 1);
}

double DateTickIterator::computeFracIndex(TimeStamp t) const {
  auto i = timeStampToIndex(t);
  std::cout << "i = " << i << std::endl;
  return i/tickSpacing();
}

AxisTick<TimeStamp> DateTickIterator::get(int index) const {
  auto ts = tickSpacing();
  auto i = index*ts;
  int year = i/12;
  int month = i % 12;
  auto date = TimeStamp::UTC(year, month+1, 1, 0, 0, 0.0);
  if (12 <= ts) {
    return AxisTick<TimeStamp>{
      date, stringFormat("%d", year)
    };
  } else {
    const char *months[] = {
        "January", "February", "March",
        "April", "May", "June",
        "July", "August", "September",
        "October", "November", "December"
    };
    return AxisTick<TimeStamp>{
      date, stringFormat("%s %d", months[month], year)
    };
  }
}

namespace {
  std::vector<int> granularitiesInMonths{
    1, 2, 3, 4, 6, 12, 24, 60, 120
  };
}

int DateTickIterator::tickSpacing() const {
  if (_level < granularitiesInMonths.size()) {
    return granularitiesInMonths[_level];
  }
  int last = granularitiesInMonths.size()-1;
  int base = granularitiesInMonths[last];
  for (int i = last; i < _level; i++) {
    base *= 10;
  }
  return base;
}















}
