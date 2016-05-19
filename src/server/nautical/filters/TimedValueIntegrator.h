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
#include <server/common/Functional.h>
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
inline double computeLambda(double left, double right, double t) {
  assert(!(right < left));
  if (left == right) {
    return 0.5;
  }
  return (t - left)/(right - left);
}

template <typename T, typename Iterator>
Array<TimedValue<T> > makeArrayFromRange(Iterator begin, Iterator end) {
  int n = computeIteratorRangeSize(begin, end);
  ArrayBuilder<TimedValue<T> > builder(n);
  for (auto i = begin; i != end; i++) {
    builder.add(*i);
  }
  return builder.get();
}

inline double toLocalTime(TimeStamp offset, TimeStamp x, const Duration<double> &unit) {
  return ((x - offset)/unit).getScalar();
}

template <typename T>
Arrayd toLocalTimes(const Array<TimedValue<T> > &values, const Duration<double> &unit) {
  if (values.empty()) {
    return Arrayd();
  }
  auto offset = values.first().time;
  return sail::map(values, [&](const TimedValue<T> &x) {
    return toLocalTime(offset, x.time, unit);
  });
}

Arrayd computeBounds(const Arrayd &localTimes);

int computeBin(const Arrayd &bounds, double x);

template <typename T>
Array<T> buildCumulative(const Arrayd &bounds,
    const Array<TimedValue<T> > &values) {
  int n = values.size();
  Array<T> cumulative(n + 1);
  cumulative[0] = values[0].value;
  for (int i = 0; i < n; i++) {
    auto width = bounds[i + 1] - bounds[i];
    cumulative[i + 1] = cumulative[i] + width*values[i].value;
  }
  return cumulative;
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
struct ValueAndDurationToNearestSample {
  // The maximum duration between one of the integration bounds
  // and the closest sample.
  // This is a measure of how useful the value is. The smaller the duration,
  // the better the quality of the value. It is the responsibility of the user
  // of this class to reject values with duration that is too high.
  Duration<double> duration;

  // The average value.
  T value;
};

template <typename T>
class TimedValueIntegrator {
public:
  typedef ValueAndDurationToNearestSample<T> Value;

  TimedValueIntegrator() {}

  static TimedValueIntegrator<T> makeFromArray(const Array<TimedValue<T> > &values) {
    assert(std::is_sorted(values.begin(), values.end()));
    if (values.empty()) {
      return TimedValueIntegrator();
    } else {
      auto times = toLocalTimes(values, unit());
      auto bounds = computeBounds(times);
      if (bounds.size() == 1) {
        assert(values.size() == 1);
        return TimedValueIntegrator<T>(values.first().time, bounds, times, Array<T>{values.first().value});
      } else {
        auto cumulative = buildCumulative(bounds, values);
        return TimedValueIntegrator<T>(values.first().time, bounds, times, cumulative);
      }
    }
  }

  template <typename Iterator>
  static TimedValueIntegrator<T> make(Iterator begin, Iterator end) {
    Array<TimedValue<T> > array = makeArrayFromRange<T>(begin, end);
    return makeFromArray(array);
  }

  Optional<Value> computeAverage(TimeStamp from, TimeStamp to) const {
    if (from == to) { // As the interval tends to zero, the average is the value at the point.
      return interpolate(from);
    } else if (to < from) { // Integrating from a high value to a low value means flipping the sign.
      auto x = computeAverage(to, from);
      if (x.defined()) {
        Value v = x.get();
        return Value{v.duration, -v.value};
      }
      return x;
    }
    if (_bounds.empty()) {
      return Optional<Value>();
    }

    auto from0 = toLocalTime(_offset, from, unit());
    auto to0 = toLocalTime(_offset, to, unit());

    if (_bounds.size() == 1) {
      auto b = _bounds.first();
      return Value{
        unit()*std::max(std::abs(b - from0), std::abs(b - to0)),
        _cumulative.first()
      };
    }

    auto maxDur = std::max(computeMaxDur(from0), computeMaxDur(to0))*unit();

    // General case
    auto toF = fitToBounds(to0);
    auto fromF = fitToBounds(from0);
    auto timeDiff = toF - fromF;

    // timeDiff being 0 can happen for two reasons:
    // (i) both of the times are outside of the defined region and on the same side.
    // (ii) They are close
    if (timeDiff > 0) {
      T valueDiff = sumTo(toF) - sumTo(fromF);
      T value = (1.0/timeDiff)*valueDiff;
      return Value{maxDur, value};
    } else {
      return interpolate(_offset + unit()*fromF);
    }
  }

  Optional<Value> interpolate(TimeStamp t) const {
    if (_bounds.empty()) {
      return Optional<Value>();
    }
    auto t0 = toLocalTime(_offset, t, unit());
    if (_bounds.size() == 1) {
      auto b = _bounds.first();
      return Value{
        unit()*std::abs(t0 - b),
        _cumulative.first()
      };
    }
    int bin = computeClosestBin(t0);
    return Value{
      unit()*computeMaxDur(t0),
      valueOfBin(bin)
    };
  }

  bool empty() const {
    return _sampleTimes.empty();
  }
private:
  TimeStamp _offset;
  Arrayd _bounds, _sampleTimes;
  Array<T> _cumulative;

  static Duration<double> unit() {return Duration<double>::seconds(1.0);}

  double fitToBounds(double localTime) const {
    return sail::clamp(localTime, _bounds.first(), _bounds.last());
  }

  TimedValueIntegrator(TimeStamp offset, Arrayd bounds, Arrayd times,
      Array<T> cumulative) :
        _offset(offset), _bounds(bounds),
        _sampleTimes(times), _cumulative(cumulative) {}

  int computeClosestBin(double t) const {
    return clamp<int>(computeBin(_bounds, t), 0, _sampleTimes.size() - 1);
  }

  double computeMaxDur(double t) const {
    return std::abs(_sampleTimes[computeClosestBin(t)] - t);
  }

  T valueOfBin(int i) const {
    return (1.0/(_bounds[i+1] - _bounds[i]))*(_cumulative[i+1] - _cumulative[i]);
  }

  T sumTo(double t) const {
    if (t <= _bounds.first()) {
      return _cumulative.first();
    } else if (_bounds.last() <= t) {
      return _cumulative.last();
    }
    int bin = computeBin(_bounds, t);
    double lambda = computeLambda(_bounds[bin], _bounds[bin+1], t);
    return (1.0 - lambda)*_cumulative[bin] + lambda*_cumulative[bin+1];
  }
};

}

#endif /* SERVER_NAUTICAL_FILTERS_TIMEDVALUEINTEGRATOR_H_ */
