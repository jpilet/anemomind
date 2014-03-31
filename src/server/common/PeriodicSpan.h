/*
 * SpacialRange.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef SPACIALRANGE_H_
#define SPACIALRANGE_H_

#include "Array.h"

namespace sail {

class PeriodicSpan {
 public:
  PeriodicSpan();
  PeriodicSpan(Arrayd values);
  virtual ~PeriodicSpan() {}
  bool intersects(const PeriodicSpan &other) const;
 private:
  double _mean, _maxDif;
};

typedef PeriodicSpan PSpan;

} /* namespace sail */

#endif /* SPACIALRANGE_H_ */
