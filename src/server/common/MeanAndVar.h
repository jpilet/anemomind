/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MEANANDVAR_H_
#define MEANANDVAR_H_

#include <ostream>
#include <server/common/Array.h>
#include <server/common/Span.h>
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

  double min() const {
    if (_count == 0) {
      return 0;
    }
    return _bounds.minv();
  }
  double max() const {
    if (_count == 0) {
      return 0;
    }
    return _bounds.maxv();
  }

 private:
  double biasedVariance() const;
  MeanAndVar(int count_, double sum, double sum2, Span<double> bounds)
    : _sum(sum), _sum2(sum2), _count(count_), _bounds(bounds) { }
  double _sum, _sum2;
  int _count;
  Span<double> _bounds;
};

std::ostream &operator<<(std::ostream &s, const MeanAndVar &x);

}

#endif /* MEANANDVAR_H_ */
