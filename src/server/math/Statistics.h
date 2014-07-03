/*
 *  Created on: 2014-03-25
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef STATISTICS_H_
#define STATISTICS_H_

#include <iosfwd>

namespace sail {

class Statistics {
 public:
  Statistics();
  void add(double value);

  double mean() const;
  double stddev() const;
  double variance() const;
  bool defined() const;
 private:
  int _count;
  double _valueSum, _value2Sum;
};

std::ostream &operator<<(std::ostream &s, const Statistics &st);

} /* namespace sail */

#endif /* STATISTICS_H_ */
