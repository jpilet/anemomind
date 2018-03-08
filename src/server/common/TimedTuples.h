/*
 * TimedTuples.h
 *
 *  Created on: 8 Mar 2018
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TIMEDTUPLES_H_
#define SERVER_COMMON_TIMEDTUPLES_H_

#include <server/common/Transducer.h>
#include <server/common/TimedValue.h>
#include <server/common/math.h>
#include <server/common/logging.h>

namespace sail {
namespace TimedTuples {

struct Settings {
  double tupleCostPerSecond = 0.001; // Just a small positive value
  int halfHistoryLength = 100; // Flush when we reach *full* history length.
};

template <typename T>
struct Indexed {
  bool defined() const {return index != -1;}

  int index = -1;
  T value;
};

template <typename T, int TupleSize>
class Stepper {
public:
  static constexpr int StateSize = staticPower(2, TupleSize);
  static constexpr double HighCost = std::numeric_limits<double>::infinity();
  static constexpr double LowCost = 0.0;

  struct BackPointer {
    int best = -1;
    double totalCost = HighCost;
  };

  typedef TimedValue<Indexed<T>> Value;

  struct State {
    Value value;
    std::array<BackPointer, StateSize> pointers;

    static State root(Value x, int stateIndex) {
      CHECK(x.value.defined());
      State dst;
      dst.value = x;
      dst.pointers[x.value.index].totalCost = LowCost;
      return dst;
    }
  };

  Stepper(const Settings& settings) :
    _settings(settings) {
    _states.push_back(State::root(Value(), 0));
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

  template <typename R>
  void apply(R* dst, Value x) {
    CHECK(x.value.defined());
    CHECK(x.time.defined());
    State state;
    state.value = x;
    CHECK(!_states.empty());
    auto last = _states.back();
    double timeCost = last.time.defined()?
        _settings.tupleCostPerSecond*((x.time - last.time).seconds())
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
        dst.cost = last.cost + timeCost;
      }
    }

  }

  template <typename R>
  void flush(R* result) {

  }
private:
  std::vector<State> _states;
  Settings _settings;
};
}
template <typename T, int TupleSize>
GenericTransducer<TimedTuples::Stepper<T, TupleSize>> timedTuples(
    const TimedTuples::Settings& settings) {
  return genericTransducer(TimedTuples::Stepper<T, TupleSize>(
      settings));
}


} /* namespace sail */

#endif /* SERVER_COMMON_TIMEDTUPLES_H_ */
