/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MEANANDVAR_H_
#define MEANANDVAR_H_

#include <ostream>
#include <server/common/Array.h>
#include <string>

namespace sail {

class MeanAndVar {
 public:
  MeanAndVar() : _sum(0), _sum2(0), _count(0) {}
  MeanAndVar(Arrayd arr);
  void add(double value);
  double mean() const;
  double variance() const;
  double rms() const;
  double standardDeviation() const;
  MeanAndVar operator+ (const MeanAndVar &other) const;
  std::string toString() const;
  int count() const {
    return _count;
  }

  MeanAndVar normalize() const;
  bool empty() const {
    return _count == 0;
  }
 private:
  double biasedVariance() const;
  MeanAndVar(int count_, double sum, double sum2) : _sum(sum), _sum2(sum2), _count(count_) {}
  double _sum, _sum2;
  int _count;
};

std::ostream &operator<<(std::ostream &s, const MeanAndVar &x);

}

#endif /* MEANANDVAR_H_ */
