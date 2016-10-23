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

namespace sail {

template <typename T>
struct AxisTick {
  T position;
  std::string tickLabel;
};

// A "tick iterator" for some type T has
// a "typedef T type;" and four methods:
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
  BasicTickIterator(int e, const std::string &unit = "");
  BasicTickIterator coarser() const;
  BasicTickIterator finer() const;
  double computeFracIndex(double value) const;
  AxisTick<double> get(int index) const;
  double tickSpacing() const;
private:
  int _exponent;
  std::string _unit;
};


// For dates. Well, we could allow for something more fine-grained,
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

template <typename T, typename TickIterator>
double tickIteratorCost(TickIterator iter, T lower, T upper) {
  double idealTickCount = 4.0;
  double width = std::abs(
      iter.computeFracIndex(lower)
      - computeFracIndex(upper));
  return std::abs(width - idealTickCount);
}

template <typename T, typename TickIterator>
struct EvaluatedTickIterator {
  TickIterator iterator;
  double cost;
  int iteration;

  EvaluatedTickIterator(
      TickIterator iter,
      T lower, T upper, int *counter) : iterator(iter),
          cost(tickIteratorCost<T, TickIterator>(iter, lower, upper)),
          iteration((*counter)++) {}

  bool operator<(const EvaluatedTickIterator<T, TickIterator> &other) const {
    return cost < other.cost;
  }
};


template <typename TickIterator>
using TypeOfTick = typename TickIterator::type;

template <typename TickIterator>
TickIterator optimizeTickIterator(
    TypeOfTick<TickIterator> lower,
    TypeOfTick<TickIterator> upper,
    TickIterator init) {
  typedef TypeOfTick<TickIterator> T;
  int counter = 0;
  EvaluatedTickIterator<T, TickIterator> best(
      init, lower, upper, &counter);
  while (true) {
    auto next = std::min(
        best, std::min(
            EvaluatedTickIterator<T, TickIterator>(
              best.iterator.finer(), lower, upper, &counter),
            EvaluatedTickIterator<T, TickIterator>(
              best.iterator.coarser(), lower, upper, &counter)));
    if (next.iteration == best.iteration ||
        next.cost == best.cost) {
      return best.iterator;
    } else {
      best = next;
    }
  }
}

/*template <typename T, typename TickIterator>
Array<AxisTick<T> > computeAxisTicks(
    T lower, T upper, TickIterator init) {
  CHECK(lower < upper);
  auto iter = optimizeTickIterator(upper - lower, init);
  auto spacing = iter.tickSpacing();
  int lowerIndex = int(ceil(lower/spacing));
  int upperIndex = int(floor(upper/spacing));
  int n = upperIndex - lowerIndex + 1;
  Array<AxisTick<T>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = iter.tick(lower + i);
  }
  return dst;
}*/

}


#endif /* SERVER_PLOT_AXISTICKS_H_ */
