/*
 * AxisTicks.h
 *
 *  Created on: 22 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_PLOT_AXISTICKS_H_
#define SERVER_PLOT_AXISTICKS_H_

#include <server/common/Array.h>
#include <cmath>
#include <string>
#include <server/common/TimeStamp.h>
#include <server/common/logging.h>

namespace sail {

template <typename T>
struct AxisTick {
  T position;
  std::string tickLabel;
};

// A "tick iterator" for some type T four methods:
//  finer() that returns a tick iterator
//    with less space between the ticks
//  coarser() that returns a tick iterator
//    with more space between the ticks
//  computeFracIndex(T x) that returns
//    a fractional tick index for x
//  get(int i) that returns an AxisTick<T> for
//    a tick with index i.

class BasicTickIterator {
public:
  typedef double type;
  BasicTickIterator(int e = 0, const std::string &unit = "");
  BasicTickIterator coarser() const;
  BasicTickIterator finer() const;
  double computeFracIndex(double value) const;
  AxisTick<double> get(int index) const;
  double tickSpacing() const;
private:
  int _exponent;
  std::string _unit;
};


// For dates. Well, we could allow for
// something more fine-grained than months,
// but due to irregular numbers of days in months, and so on,
// I am not going to take on that challenge now.
// 1 month, 2 months 3 months 6 months, 1 year, ....
class DateTickIterator {
public:
  typedef TimeStamp type;
  DateTickIterator(int l = 0);
  DateTickIterator finer() const;
  DateTickIterator coarser() const;
  double computeFracIndex(TimeStamp t) const;
  AxisTick<TimeStamp> get(int index) const;
  int tickSpacing() const;
private:
  int _level;
};

template <typename T>
class PhysicalQuantityIterator {
public:
  typedef T type;
  PhysicalQuantityIterator(int level = 0);
  PhysicalQuantityIterator<T> finer() const;
  PhysicalQuantityIterator<T> coarser() const;
  double computeFracIndex(T x) const;
  T tickSpacing() const;
  AxisTick<T> get(int i) const;
private:
  PhysicalQuantityIterator(const BasicTickIterator &i);
  BasicTickIterator _iter;
};

template <typename TickIterator>
using TypeOfTick =
    decltype(std::declval<TickIterator>().get(0).position);

template <typename TickIterator>
double tickIteratorCost(TickIterator iter,
    TypeOfTick<TickIterator> lower,
    TypeOfTick<TickIterator> upper) {
  double idealTickCount = 8.0;
  double width = std::abs(
      iter.computeFracIndex(lower)
      - iter.computeFracIndex(upper));
  return std::abs(width - idealTickCount);
}

template <typename TickIterator>
struct EvaluatedTickIterator {
  TickIterator iterator;
  double cost;
  int iteration;

  EvaluatedTickIterator(
      TickIterator iter,
      TypeOfTick<TickIterator> lower,
      TypeOfTick<TickIterator> upper,
      int *counter) : iterator(iter),
          cost(tickIteratorCost<TickIterator>(iter, lower, upper)),
          iteration((*counter)++) {}

  bool operator<(const EvaluatedTickIterator<TickIterator> &other) const {
    return cost < other.cost;
  }
};



template <typename TickIterator>
TickIterator optimizeTickIterator(
    TypeOfTick<TickIterator> lower,
    TypeOfTick<TickIterator> upper,
    TickIterator init) {
  typedef TypeOfTick<TickIterator> T;
  int counter = 0;
  EvaluatedTickIterator<TickIterator> best(
      init, lower, upper, &counter);
  while (true) {
    auto next = std::min(
        best, std::min(
            EvaluatedTickIterator<TickIterator>(
              best.iterator.finer(), lower, upper, &counter),
            EvaluatedTickIterator<TickIterator>(
              best.iterator.coarser(), lower, upper, &counter)));
    if (next.iteration == best.iteration ||
        next.cost == best.cost) {
      return best.iterator;
    } else {
      best = next;
    }
  }
}

template <typename TickIterator>
Array<AxisTick<TypeOfTick<TickIterator>>> computeAxisTicks(
    TypeOfTick<TickIterator> lower,
    TypeOfTick<TickIterator> upper,
    TickIterator init) {
  CHECK(lower < upper);
  auto iter = optimizeTickIterator(lower, upper, init);
  int lowerIndex = int(floor(iter.computeFracIndex(lower)));
  int upperIndex = int(ceil(iter.computeFracIndex(upper)));
  int n = upperIndex - lowerIndex + 1;
  Array<AxisTick<TypeOfTick<TickIterator>>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = iter.get(lowerIndex + i);
  }
  return dst;
}

}


#endif /* SERVER_PLOT_AXISTICKS_H_ */
