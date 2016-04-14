/*
 * TimedValueIntegrator.h
 *
 *  Created on: Apr 14, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_TIMEDVALUEINTEGRATOR_H_
#define SERVER_NAUTICAL_FILTERS_TIMEDVALUEINTEGRATOR_H_

#include <server/common/ArrayBuilder.h>
#include <server/common/TimedValue.h>
#include <server/common/Optional.h>
#include <algorithm>

namespace sail {

template <typename Iter>
int computeIteratorRangeSize(Iter begin, Iter end) {

  // TODO: Use some template magic to figure
  // out if it is OK to just return "end - begin".

  int counter = 0;
  for (auto i = begin; i != end; i++) {
    counter++;
  }
  return counter;
}

// Returns an interpolation factor in [0, 1]
double computeLambda(TimeStamp left, TimeStamp right, TimeStamp t) {
  assert(!(right < left));
  if (left == right) {
    return 0.5;
  }
  return (t - left)/(right - left);
}

/*
 * This class models the underlying signal using nearest neighbour
 * interpolation. So the value of point X is the value of the closest sample.
 * The class also allows for integration of multiple samples, over any time span.
 * This is done using a precomputed integral.
 *
 * If we were to instead use linear interpolation between the samples, we would
 * expect smoother results. But this is also more job to implement, especially
 * when computing the integral of a linearly interpolated signal. I think this
 * is good enought for now.
 *
 * NOTE: it is probably a bad idea to filter angles using this
 * class, because they are cyclic. It is better to first map the angles
 * to unit vectors, and then filter those vectors.
 *
 * And also, since an angle often goes together with a velocity,
 * it can be a good idea to construct horizontal motions that are
 * filtered, instead of velocity and angle separately.
 *
 * TYPICAL USE CASE: We are given a noisy signal, possibly irregularly sampled,
 * we don't know the sampling rate of it. It could be anything, such as 100 Hz or
 * 0.3 Hz. We want to sample it at a rate of, say, 1 Hz. To do this, we use the
 * 'computeAverage' method to compute the signal average over intervals starting
 * from some time T, such as, [T + 0 seconds, T + 1 seconds],
 * [T + 1 seconds, T + 2 seconds], [T + 2 seconds, T + 3] seconds. That way
 * we avoid aliasing effects if the sampling rate of the underlying signal is high.
 * And if it is low, we just get nearest neighbour interpolation instead.
 *
 */
template <typename T>
class TimedValueIntegrator {
public:
  template <typename Iterator>
  static TimedValueIntegrator<T> make(Iterator begin, Iterator end) {
    assert(std::is_sorted(begin, end));

    auto n = computeIteratorRangeSize(begin, end);
    ArrayBuilder<TimeStamp> times(n);
    ArrayBuilder<T> cumulative(n+1);
    if (0 < n) {
      auto x = begin->value;
      cumulative.add(x);
      for (auto iter = begin; iter != end; iter++) {
        times.add(iter->time);
        x = x + iter->value;
        cumulative.add(x);
      }
    }
    assert(times.size() + 1 == cumulative.size() || n == 0);
    return TimedValueIntegrator<T>(times.get(), cumulative.get());
  }

  struct Value {
    // The maximum duration between one of the integration bounds
    // and the closest sample.
    // This is a measure of how useful the value is. The smaller the duration,
    // the better the quality of the value. It is the responsibility of the user
    // of this class to reject values with duration that is too high.
    Duration<double> maxDuration;

    // The average value.
    T value;
  };

  Optional<Value> computeAverage(TimeStamp from, TimeStamp to) const {
    if (_cumulative.empty()) {
      return Optional<Value>();
    }

    Index fromX = computeIndex(from);
    Index toX = computeIndex(to);
    if (from == to || std::abs(fromX.index - toX.index) < 0.001) {
      auto value = computeAtPoint(fromX.index);
      return Value{fromX.closestDur, value};
    } else {
      auto fromValue = sumToIndex(fromX.index);
      auto toValue = sumToIndex(toX.index);
      double factor = (1.0/(toX.index - fromX.index));
      return Value{
          std::max(fromX.closestDur, toX.closestDur),
          factor*(toValue - fromValue)};
    }
  }

  Optional<Value> interpolate(TimeStamp t) const {
    if (_cumulative.empty()) {
      return Optional<Value>();
    }
    auto x = computeIndex(t);
    auto value = computeAtPoint(x.index);
    return Value{x.closestDur, value};
  }
private:
  struct Index {
    // Duration
    Duration<double> closestDur;
    double index;
  };

  Index computeIndex(const TimeStamp &t) const {
    assert(!_times.empty()); // Checked by computeAverage
    auto it = lowerBound(t);
    if (it == _times.begin()) {
      auto maxDur = _times.first() - t;
      return Index{maxDur, 0.0};
    } else if (it == _times.end()) {
      auto maxDur = t - _times.last();
      return Index{maxDur, width()};
    } else {
      auto a = *(it - 1);
      auto b = *it;
      auto prev = it - 1;
      double lambda = computeLambda(a, b, t);
      double index = (prev - _times.begin()) + lambda;
      double itgIndex = index + 0.5;
      return Index{std::min(t - a, b - t), index};
    }
  }

  struct ItgIndex {
    ItgIndex(double index0) {
      index = index0 + 0.5;
      floored = floor(index);
      i = int(floored);
    }
    double index;
    double floored;
    int i;
  };

  int getSide(int index) const {
    if (index < 0) {
      return -1;
    } else if (_cumulative.size() - 1 <= index) {
      return 1;
    }
    return 0;
  }

  T computeAtPoint(double index0) const {
    ItgIndex at(index0);
    switch (getSide(at.i)) {
    case -1:
      return _cumulative[1] - _cumulative[0];
    case 1: {
      auto n = _cumulative.size();
      return _cumulative[n-1] - _cumulative[n-2];
    }
    default:
      return _cumulative[at.i+1] - _cumulative[at.i];
    };
  }

  T sumToIndex(double index0) const {
    ItgIndex at(index0);
    switch (getSide(at.i)) {
    case -1:
      return _cumulative.first();
    case 1:
      return _cumulative.last();
    default:
      double lambda = at.index - at.floored;
      return (1.0 - lambda)*_cumulative[at.i] + lambda*_cumulative[at.i + 1];
    }
  }


  TimedValueIntegrator(const Array<TimeStamp> &times,
      const Array<T> &cumulative)
    : _times(times), _cumulative(cumulative) {}


  const TimeStamp *lowerBound(const TimeStamp &t) const {
    return std::lower_bound(_times.begin(), _times.end(), t);
  }

  int sampleCount() const {
    return _times.size();
  }

  double width() const {
    return _times.size() - 1;
  }

  Array<T> _cumulative;
  Array<TimeStamp> _times;
};


}

#endif /* SERVER_NAUTICAL_FILTERS_TIMEDVALUEINTEGRATOR_H_ */
