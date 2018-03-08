/*
 * TimedTuples.h
 *
 *  Created on: 8 Mar 2018
 *      Author: jonas
 *
 *
 *
 *  Dynamic-programming based algorithm that forms tuples of timed
 *  values from different channels. The objective is to form
 *  as many tuples as possible, and every tuple being as compact
 *  in time as possible. To limit memory consumption, a parameter
 *  can be set that controls the trade-off between accuracy of the
 *  solution and memory usage.
 *
 *  The algorithm itself is implemented as a transducer, that accepts
 *  incoming timed values wrapping type Indexed<T> where T can probably
 *  be a boost::variant holding the actual value. The 'index' in the Indexed<T>
 *  type is used to identify the source channel, and is also the index
 *  of that value in the output tuple.
 *
 */

#ifndef SERVER_COMMON_TIMEDTUPLES_H_
#define SERVER_COMMON_TIMEDTUPLES_H_

#include <server/common/Transducer.h>
#include <server/common/TimedValue.h>
#include <server/common/math.h>
#include <server/common/logging.h>
#include <array>
#include <iostream>

namespace sail {
namespace TimedTuples {

struct Settings {
  // Just a small positive value, so that we prefer tuples
  // spanning a short time span.
  double tupleCostPerSecond = 0.001;

  // This parameter is used to limit the memory consumption,
  // at the expense of an approximate solution. We have a lot of
  // memory, so this parameter can be high.
  int halfHistoryLength = 1000; // Flush when we reach *full* history length.
};

template <typename T>
struct Indexed {
  bool defined() const {return index != -1;}

  int index = -1;
  T value;

  Indexed(int i, T v) : index(i), value(v) {}
  Indexed() {}
};

template <typename T, int TupleSize>
class Stepper {
public:
  static constexpr int StateSize = staticPower(2, TupleSize)-1;
  static constexpr double HighCost = std::numeric_limits<double>::infinity();
  static constexpr double LowCost = 0.0;

  struct BackPointer {
    int best = -1;
    double cost = HighCost;
  };

  typedef TimedValue<Indexed<T>> Value;

  struct State {
    int optimized = -1;
    Value value;
    std::array<BackPointer, StateSize> pointers;

    static State root(int stateIndex, Value x = Value()) {
      State dst;
      dst.value = x;
      dst.pointers[stateIndex].cost = LowCost;
      return dst;
    }
  };

  Stepper(const Settings& settings) :
    _settings(settings) {
    _states.push_back(State::root(0));
  }

  struct Cand {
    int index = -1;
    double cost = 0.0;

    Cand() {}
    Cand(int i, double c) : index(i), cost(c) {}

    bool operator<(const Cand& other) const {
      return cost < other.cost;
    }
  };

  void addValue(Value x) {
    CHECK(x.value.defined());
    CHECK(0 <= x.value.index && x.value.index < TupleSize);
    CHECK(x.time.defined());
    State state;
    state.value = x;
    CHECK(!_states.empty());
    auto last = _states.back();
    double timeCost = last.value.time.defined()?
        _settings.tupleCostPerSecond*((x.time - last.value.time).seconds())
        : 0.0;

    for (int i = 1; i < StateSize; i++) {
      std::bitset<TupleSize> bits(i);
      auto& dst = state.pointers[i];
      if (bits.test(x.value.index)) {
        auto pred = bits;
        pred.set(x.value.index, false);
        int predIndex = pred.to_ulong();
        auto best = std::min(
            Cand(predIndex, last.pointers[predIndex].cost),
            Cand(i, last.pointers[i].cost));
        dst.best = best.index;
        dst.cost = best.cost + timeCost;
      } else {
        dst.best = i;
        dst.cost = last.pointers[i].cost + timeCost;
      }
    }{ // Special treatment for state 0.
      std::bitset<TupleSize> bits;
      bits.set();
      bits.set(x.value.index, false);
      int index = bits.to_ulong();
      constexpr double tupleCompletionReward = 1.0;
      auto best = std::min(
          Cand(index, last.pointers[index].cost
              + timeCost - tupleCompletionReward),
          Cand(0, last.pointers[0].cost));
      auto& dst = state.pointers[0];
      dst.best = best.index;
      dst.cost = best.cost;
    }
    _states.push_back(state);
  }

  template <typename R>
  void apply(R* result, Value x) {
    addValue(x);
    if (_states.size() > 2*_settings.halfHistoryLength) {
      flushTo<R>(result, _settings.halfHistoryLength);
    }
  }

  void dispStates() {
    for (int i = 0; i < _states.size(); i++) {
      const auto& s = _states[i];
      std::cout << "State: " << s.value.value.index;
      std::cout << "  Pointers: ";
      for (int j = 0; j < StateSize; j++) {
        std::cout << "   j=" << j
            << " c=" << s.pointers[j].cost
            << " p=" << s.pointers[j].best;
      }
      std::cout << "  opt=" << s.optimized;
      std::cout << std::endl;
    }
  }

  template <typename R>
  void flush(R* result) {
    flushTo<R>(result, _states.size());
  }

  template <typename R>
  void flushTo(R* result, int n) {
    //std::cout << "Flush " << n << "/" << _states.size() << std::endl;
    traceAll();
    for (int i = 0; i < n; i++) {
      const auto& state = _states[i];
      if (state.value.value.defined()) {
        _tupleInProgress[state.value.value.index] =
            TimedValue<T>(
                state.value.time,
                state.value.value.value);
      }
      if (state.optimized == 0 && i > 0 && _states[i-1].optimized != 0) {
        result->add(_tupleInProgress);
      }
    }
    if (n < _states.size()) {
      std::vector<State> backup(_states.begin() + n, _states.end());
      auto root = State::root(_states[n-1].optimized);
      _states.resize(1);
      _states[0] = root;
      for (auto x: backup) {
        addValue(x.value);
      }
    }
  }

  int bestTermination() const {
    const auto& last = _states.back();
    Cand cand(-1, HighCost);
    for (int i = 0; i < StateSize; i++) {
      cand = std::min(cand, Cand(i, last.pointers[i].cost));
    }
    return cand.index;
  }

  void traceAll() {
    int n = _states.size();
    auto at = bestTermination();
    for (int i = n-1; i >= 0; i--) {
      _states[i].optimized = at;
      at = _states[i].pointers[at].best;
    }
  }
private:
  std::array<TimedValue<T>, TupleSize> _tupleInProgress;
  std::vector<State> _states;
  Settings _settings;
};
}

/**
 *
 * This is the function used to construct the TimedTuples transducer.
 *
 * That transducer takes TimedValue<Indexed<T>> as input, and outputs
 * std::array<TimedValue<T>, TupleSize> elements of the formed tuples.
 *
 * We can let T be a boost::variant if we are dealing with many
 * different types.
 *
 */
template <typename T, int TupleSize>
GenericTransducer<TimedTuples::Stepper<T, TupleSize>> trTimedTuples(
    const TimedTuples::Settings& settings = TimedTuples::Settings()) {
  return genericTransducer(TimedTuples::Stepper<T, TupleSize>(
      settings));
}


} /* namespace sail */

#endif /* SERVER_COMMON_TIMEDTUPLES_H_ */
