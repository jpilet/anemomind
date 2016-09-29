/*
 * StateReconstructor.h
 *
 *  Created on: 22 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_
#define SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_

#include <unordered_map>
#include <map>
#include <server/common/Span.h>
#include <server/common/TimeStamp.h>
#include <server/nautical/BoatState.h>
#include <server/common/TimedValue.h>
#include <server/nautical/calib/BoatParameters.h>
#include <server/common/Array.h>

namespace sail {

#define FOREACH_MEASURE_TO_CONSIDER(OP) \
  OP(AWA) \
  OP(AWS) \
  OP(MAG_HEADING) \
  OP(WAT_SPEED) \
  OP(ORIENT)

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
    int index = int(round((t - offset)/period));
    return 0 <= index && index < sampleCount? index : -1;
  }
};

template <typename T>
void foreachSpan(const TimeStampToIndexMapper &mapper,
    const Array<TimedValue<T> > &values,
    std::function<void(int, Spani)> cb);

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

  std::map<std::string, int> sensorIndices;
  std::vector<std::pair<int, Spani> > valuesPerIndex;
  std::vector<TaggedValue> values;
};

template <typename T>
ValueAccumulator<T> makeValueAccumulator(
  const TimeStampToIndexMapper &mapper,
  const std::map<std::string, Array<TimedValue<T> > > &srcData);


template <typename T>
struct ReconstructionDataFit;

template <typename BoatStateSettings>
class BoatStateReconstructor {
public:
  BoatStateReconstructor(
      const Array<BoatState<double> > &initialStates,
      const TimeStampToIndexMapper &mapper,
      const CalibDataChunk &chunk) :
/*#define INITIALIZE_VALUE_ACC(HANDLE) \
  HANDLE(mapper, chunk.HANDLE),
FOREACH_MEASURE_TO_CONSIDER(INITIALIZE_VALUE_ACC)
#undef INITIALIZE_VALUE_ACC*/
        _timeMapper(mapper),
        _initialStates(initialStates) {
    CHECK(initialStates.size() == mapper.sampleCount);
  }

  /*Array<BoatState<double> > reconstruct(
      const BoatParameters<double> &sensorParams) const {
    //ceres::Problem problem;
  }*/
#define DECLARE_VALUE_ACC(HANDLE) \
  ValueAccumulator<typename TypeForCode<HANDLE>::type> HANDLE;
  FOREACH_MEASURE_TO_CONSIDER(DECLARE_VALUE_ACC)
#undef DECLARE_VALUE_ACC
private:
  TimeStampToIndexMapper _timeMapper;
  Array<BoatState<double> > _initialStates;
};

}



#endif /* SERVER_NAUTICAL_CALIB_BOATSTATERECONSTRUCTOR_H_ */
