/*
 * RealPerfSurf.h
 *
 *  Created on: 2 Mar 2018
 *      Author: jonas
 *
 * This is glue code needed to perform a full polar computation
 * on a dataset.
 *
 */

#ifndef SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_
#define SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_

#include <server/common/Transducer.h>
#include <server/common/TimedValue.h>
#include <iostream>

namespace sail {

template <typename Iterator, typename Right>
class TimedValuePairsStep {
public:
  // Obs: butEnd is a *valid iterator*. We can dereference it.
  // It is the iterator pointing just before the end of the range.
  TimedValuePairsStep(Iterator begin, Iterator butEnd)
    : _begin(begin), _butEnd(butEnd) {}


  template <typename Result>
  void apply(Result* r, TimedValue<Right> x) {
    if (_begin >= _butEnd || x.time < _begin->time) {
      return;
    }

    // Advance
    while (_begin < _butEnd && (_begin+1)->time <= x.time) {
      _begin++;
      addPending(r);
      _counter = 0;
    }
    if (_begin == _butEnd) {
      return;
    }

    if (_counter == 0) {
      r->add(std::make_pair(*_begin, x));
    }
    _pending = x;

    _counter++;
  }

  template <typename Result>
  void flush(Result* r) {
    if (_begin < _butEnd) {
      _begin++;
      addPending(r);
    }
    r->flush();
  }
private:
  template <typename R>
  void addPending(R* r) {
    if (0 < _counter) {
      r->add(std::make_pair(*_begin, _pending));
    }
  }
  TimedValue<Right> _pending;
  int _counter = 0;
  Iterator _begin, _butEnd;
};

template <typename Right, typename Iterator>
GenericTransducer<TimedValuePairsStep<Iterator, Right>> trTimedValuePairs(
    Iterator b, Iterator e0) {
  auto butEnd = e0;
  butEnd--;
  return genericTransducer(TimedValuePairsStep<Iterator, Right>(b, butEnd));
}

class IsTightTimePair {
public:
  IsTightTimePair(Duration<double> d) : _maxDur(d) {}

  template <typename A, typename B>
  bool operator()(const std::pair<TimedValue<A>, TimedValue<B>>& p) const {
    return fabs(p.first.time - p.second.time) < _maxDur;
  }
private:
  Duration<double> _maxDur;
};

class CollapseTimePair {
public:
  template <typename A, typename B>
  TimedValue<std::pair<A, B>> operator()(
      const std::pair<TimedValue<A>, TimedValue<B>>& pair) const {
    auto t = pair.first.time + 0.5*(pair.second.time - pair.first.time);
    return TimedValue<std::pair<A, B>>(
        t, {pair.first.value, pair.second.value});
  }
};

struct WindAndBoatSpeedSample {
  Velocity<double> tws;
  Angle<double> twa;
  Velocity<double> boatSpeed;
};


template <typename TwsColl, typename TwaColl, typename SpeedColl>
Array<TimedValue<WindAndBoatSpeedSample>> buildWindAndBoatSpeedSamples(
    const TwsColl& twsColl,
    const TwaColl& twaColl,
    const SpeedColl& speedColl,
    Duration<double> pairThreshold) {
  return transduce(
      twsColl,
      trTimedValuePairs<Velocity<double>>(twaColl.begin(), twaColl.end())
      |
      trFilter(IsTightTimePair(pairThreshold))
      |
      trMap(CollapseTimePair())
      |
      trTimedValuePairs<std::pair<Angle<double>, Velocity<double>>>(
          speedColl.begin(), speedColl.end())
      |
      trFilter(IsTightTimePair(pairThreshold))
      |
      trMap(CollapseTimePair())
      |
      trMap([](const TimedValue<
          std::pair<Velocity<double>,
          std::pair<Angle<double>, Velocity<double>>>>& x) {
        WindAndBoatSpeedSample dst;
        dst.boatSpeed = x.value.first;
        dst.twa = x.value.second.first;
        dst.tws = x.value.second.second;
        return TimedValue<WindAndBoatSpeedSample>(x.time, dst);
      }),
      IntoNowhere());

  /*return transduce(
      twsColl,
      trTimedValuePairs(twaColl.begin(), twaColl.end())
      | // <-- (twa, tws)
      trFilter(IsTightTimePair(pairThreshold))
      |
      trMap(CollapseTimePair())
      |
      trTimedValuePairs(speedColl.begin(), speedColl.end())
      | // <-- (boat-speed, (twa, tws))
      trFilter(IsTightTimePair(pairThreshold))
      |
      trMap(CollapseTimePair())
      |
      trMap([](const TimedValue<
          std::pair<Velocity<double>,
            std::pair<Angle<double>, Velocity<double>>>>& x) {
        WindAndBoatSpeedSample dst;
        dst.boatSpeed = x.value.first;
        dst.twa = x.value.second.first;
        dst.tws = x.value.second.second;
        return TimedValue<WindAndBoatSpeedSample>(x.time, dst);
      }),
      IntoArray<TimedValue<WindAndBoatSpeedSample>>());*/

}


} /* namespace sail */

#endif /* SERVER_NAUTICAL_TGTSPEED_REALPERFSURF_H_ */
