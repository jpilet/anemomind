/*
 * StateReconstructor.h
 *
 *  Created on: 22 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_
#define SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_

#include <server/nautical/calib/Fitness.h>
#include <server/math/BandedLevMar.h>
#include <unordered_map>

namespace sail {

// A CalibDataChunk are measurements that are grouped together
struct CalibDataChunk {
  Array<TimedValue<GeographicPosition<double>>> filteredPositions;
#define MAKE_DATA_MAP(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, Array<TimedValue<TYPE>>> HANDLE;
FOREACH_CHANNEL(MAKE_DATA_MAP)
#undef MAKE_DATA_MAP
};

struct TimeStampToIndexMapper {
public:
  TimeStamp offset;
  Duration<double> period;
  int sampleCount;

  int map(TimeStamp t) const {
    int index = int(floor((t - offset)/period));
    return 0 <= index && index < sampleCount? index : -1;
  }
};

template <typename T>
void foreachSpan(const TimeStampToIndexMapper &mapper,
    const Array<TimedValue<T> > &values,
    std::function<void(int, Spani)> cb) {
  assert(std::is_sorted(values.begin(), values.end()));
  int currentPosition = 0;
  while (currentPosition < values.size()) {
    int currentIndex = mapper.map(values[currentPosition].time);
    int nextPosition = currentPosition + 1;
    while (nextPosition < values.size()) {
      int nextIndex = mapper.map(values[nextPosition].time);
      if (nextIndex != currentIndex) {
        break;
      }
      nextPosition++;
    }
    if (currentIndex != -1) {
      cb(currentIndex, Spani(currentPosition, nextPosition));
    }
    currentPosition = nextPosition;
  }
}

// A memory- and cache efficient data structure
// used by the optimizer
template <typename T>
struct ValueAccumulator {
  struct TaggedValue {
    int sensorIndex;
    int sampleIndex;
    T value;

    bool operator<(const TaggedValue &other) const {
      return sampleIndex < other.sampleIndex;
    }
  };

  ValueAccumulator(
    const TimeStampToIndexMapper &mapper,
    const std::map<std::string, Array<TimedValue<T> > > &srcData) {
    int sensorCounter = 0;
    int sampleCounter = 0;
    for (auto kv: srcData) {
      sensorIndices[kv.first] = sensorCounter++;
      foreachSpan<T>(mapper, kv.second, [&](int sampleIndex, Spani span) {
        sampleCounter += span.width();
      });
    }
    values.reserve(sampleCounter);

    assert(sensorCounter == sensorIndices.size());
    for (auto kv: srcData) {
      assert(sensorIndices.count(kv.first) == 1);
      int sensorIndex = sensorIndices[kv.first];
      foreachSpan<T>(mapper, kv.second, [&](int sampleIndex, Spani span) {
        for (auto i: span) {
          values.push_back(TaggedValue{
            sensorIndex, sampleIndex, kv.second[i].value});
        }
      });
    }
    std::unordered_map<int, Spani> tmp;
    assert(values.size() == sampleCounter);
    std::sort(values.begin(), values.end());
    for (int i = 0; i < values.size(); i++) {
      int index = values[i].sampleIndex;
      auto f = tmp.find(index);
      if (f == tmp.end()) {
        tmp.insert(std::pair<int, Spani>(index, Spani(i, i+1)));
      } else {
        f->second.extend(i);
        f->second.extend(i+1);
      }
    }
    for (auto kv: tmp) {
      valuesPerIndex.push_back(kv);
    }
    std::sort(valuesPerIndex.begin(), valuesPerIndex.end(),
        [](const std::pair<int, Spani> &a,
            const std::pair<int, Spani> &b) {
      return a.first < b.first;
    });
  }

  std::map<std::string, int> sensorIndices;
  std::vector<std::pair<int, Spani> > valuesPerIndex;
  std::vector<TaggedValue> values;
};


template <typename BoatStateSettings>
class BoatStateReconstructor {
public:
  BoatStateReconstructor(
      const TimeStampToIndexMapper &mapper,
      const CalibDataChunk &chunk) :
#define INITIALIZE_VALUE_ACC(HANDLE) \
  HANDLE(mapper, chunk.HANDLE),
FOREACH_MEASURE_TO_CONSIDER(INITIALIZE_VALUE_ACC)
#undef INITIALIZE_VALUE_ACC
        _timeMapper(mapper) {}

  template <typename T>
  Array<BoatState<T> > reconstruct(
      const SensorDistortionSet<T> &sensorParams) const {
    using namespace BandedLevMar;
    Problem<T> problem;
    //runLevMar();
  }
private:
  TimeStampToIndexMapper _timeMapper;
#define DECLARE_VALUE_ACC(HANDLE) \
  ValueAccumulator<typename TypeForCode<HANDLE>::type> HANDLE;
  FOREACH_MEASURE_TO_CONSIDER(DECLARE_VALUE_ACC)
#undef DECLARE_VALUE_ACC
};


}



#endif /* SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_ */
