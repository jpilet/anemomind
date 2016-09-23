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

namespace sail {

template <typename T>
struct ValueAccumulator {
  struct ValueGroup {
    int sensorIndex;
    int sampleIndex;
    Spani span;
  };
  std::map<std::string, int> sensorIndices;
  std::vector<Spani> valueGroupPerIndex;
  std::vector<ValueGroup> valueGroups;
  std::vector<T> allValues;
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
    const Array<TimedValue<T> > &values, std::function<void(Spani)> cb) {
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
      cb(Spani(currentPosition, nextPosition));
    }
  }
}

/*template <typename T>
ValueAccumulator<T> makeValueAccumulator(
    const TimeStampToIndexMapper &mapper,
    const std::map<std::string, Array<TimedValue<T> > > &srcData) {
  ValueAccumulator<T> dst;
  int sensorCounter = 0;
  int sampleCounter = 0;
  int groupCounter = 0;
  for (auto kv: srcData) {
    dst.sensorIndices[kv.first] = sensorCounter++;
    foreachSpan<T>(mapper, kv.second, [&](int sampleIndex, Spani span) {
      groupCounter++;
      sampleCounter += span.width();
    });
  }
  dst.valueGroupPerIndex.reserve(mapper.sampleCount);
  dst.valueGroups.reserve(groupCounter);
  dst.allValues.reserve(sampleCounter);

  assert(sensorCounter == dst.sensorIndices.size());
  for (auto kv: srcData) {
    assert(dst.sensorIndices.count(kv.first) == 1);
    int sensorIndex = dst.sensorIndices[kv.first];
    foreachSpan<T>(mapper, kv.second, [&](int sampleIndex, Spani span) {
      int from = dst.allValues.size();
      for (auto i: span) {
        dst.allValues.push_back(kv.second[i]);
      }
      int to = dst.allValues.size();
      dst.valueGroups.push_back();
    });
  }
}*/



template <typename T, typename BoatStateSettings>
class BoatStateReconstructor {
public:
  BoatStateReconstructor(
      TimeStamp offsetTime,
      Duration<double> samplingPeriod,
      const Array<BoatState<T> > &base) :
        _offsetTime(offsetTime),
        _samplingPeriod(samplingPeriod),
        _base(base) {}

  /*template <DataCode code>
  void addObservation(
      TimedValue<typename TypeForCode<code>::type> &x) {
    double realIndex = (x.time - _offsetTime)/_samplingPeriod;
    int index = int(round(realIndex));
    const int inputCount = BoatStateParamCount<BoatStateSettings>::value;
    if (0 <= index && index < sampleCount()-1) {
      int from = index*inputCount;
      int to = from + 2*inputCount;
      _problem.addCost(
          Spani(from, to),
          new BoatStateFitness<code, BoatStateSettings>(
              realIndex, x.value,
              _base.ptr() + index));

    }
  }*/

  int sampleCount() const {
    return _base.size();
  }
private:
  TimeStamp _offsetTime;
  Duration<double> _samplingPeriod;
  Array<BoatState<double> > _base;
  BandedLevMar::Problem<T> _problem;
};


}



#endif /* SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_ */
