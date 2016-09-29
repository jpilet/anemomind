/*
 * BoatStateReconstructor.cpp
 *
 *  Created on: 29 Sep 2016
 *      Author: jonas
 */

#include <server/nautical/calib/BoatStateReconstructor.h>
#include <server/nautical/calib/Fitness.h>

namespace sail {


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

// Forced instantiation for unit tests.
template void foreachSpan<Velocity<double>>(
    const TimeStampToIndexMapper &mapper,
    const Array<TimedValue<Velocity<double>> > &values,
    std::function<void(int, Spani)> cb);

template <typename T>
ValueAccumulator<T> makeValueAccumulator(
  const TimeStampToIndexMapper &mapper,
  const std::map<std::string, Array<TimedValue<T> > > &srcData) {
  typedef ValueAccumulator<T> DstType;
  DstType dst;
  int sensorCounter = 0;
  int sampleCounter = 0;
  for (auto kv: srcData) {
    dst.sensorIndices[kv.first] = sensorCounter++;
    foreachSpan<T>(mapper, kv.second, [&](int sampleIndex, Spani span) {
      sampleCounter += span.width();
    });
  }
  dst.values.reserve(sampleCounter);

  assert(sensorCounter == dst.sensorIndices.size());
  for (auto kv: srcData) {
    assert(dst.sensorIndices.count(kv.first) == 1);
    int sensorIndex = dst.sensorIndices[kv.first];
    foreachSpan<T>(mapper, kv.second, [&](int sampleIndex, Spani span) {
      for (auto i: span) {
        auto tagged = typename DstType::TaggedValue{
          sensorIndex, sampleIndex, kv.second[i].value};
        dst.values.push_back(tagged);
      }
    });
  }
  std::unordered_map<int, Spani> tmp;
  assert(dst.values.size() == sampleCounter);
  std::sort(dst.values.begin(), dst.values.end());
  for (int i = 0; i < dst.values.size(); i++) {
    int index = dst.values[i].sampleIndex;
    auto f = tmp.find(index);
    if (f == tmp.end()) {
      tmp.insert(std::pair<int, Spani>(index, Spani(i, i+1)));
    } else {
      f->second.extend(i);
      f->second.extend(i+1);
    }
  }
  for (auto kv: tmp) {
    dst.valuesPerIndex.push_back(kv);
  }
  std::sort(dst.valuesPerIndex.begin(), dst.valuesPerIndex.end(),
      [](const std::pair<int, Spani> &a,
          const std::pair<int, Spani> &b) {
    return a.first < b.first;
  });
  return dst;
}

// Forced instantiations for unit tests.
template ValueAccumulator<Angle<double>> makeValueAccumulator<Angle<double>>(
  const TimeStampToIndexMapper &mapper,
  const std::map<std::string, Array<TimedValue<Angle<double>> > > &srcData);
template ValueAccumulator<Velocity<double>> makeValueAccumulator<Velocity<double>>(
  const TimeStampToIndexMapper &mapper,
  const std::map<std::string, Array<TimedValue<Velocity<double>> > > &srcData);



}


